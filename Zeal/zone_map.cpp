#include "zone_map.h"
#include "EqPackets.h"
#include "Zeal.h"
#include "EqAddresses.h"
#include "string_util.h"
#include "zone_map_data.h"

// Possible enhancements:
// - Preserve marker across rect size changes
// - Support a small / med / large map defaults command
// - Support zoom and pan
// - Do something with map data points of interest (or purge to reduce memory/map sizes)

static constexpr int kMarkerCount = 4;  // Four triangles.
static constexpr int kPositionCount = 2;  // Two triangles.
static constexpr int kPositionVertices = kPositionCount * 3; // Fixed triangle list.

void ZoneMap::reset() {
    release_resources();

    if (enabled) {
        Zeal::EqGame::print_chat("Map: Disabled");
    }
    enabled = false;
    zone_id = kInvalidZoneId;
    position = Vec3(0, 0, 0);
    heading = 0;

    line_count = 0;
    line_buffer_size = 0;
}

void ZoneMap::release_resources() {
    if (line_vertex_buffer) {
        line_vertex_buffer->Release();
        line_vertex_buffer = nullptr;
    }
    if (position_vertex_buffer) {
        position_vertex_buffer->Release();
        position_vertex_buffer = nullptr;
    }
    if (marker_vertex_buffer) {
        marker_vertex_buffer->Release();
        marker_vertex_buffer = nullptr;
    }
}

void ZoneMap::load_map() {
    IDirect3DDevice8* device = ZealService::get_instance()->dx->GetDevice();
    if (!device)
        return;

    release_resources();

    const ZoneMapData* zone_map_data = get_zone_map_data(zone_id);
    if (!zone_map_data) {
        Zeal::EqGame::print_chat(USERCOLOR_SPELL_FAILURE, "Error: failed to load map zone_id: %i", zone_id);
        return;
    }
    Zeal::EqGame::print_chat("Map: Loaded zone %s", zone_map_data->name);

    // Notes on scaling the map_data world coordinates to map screen window coordinates.
    //   The map data coordinate system is in the game world scale but negated y and x values.
    //     map_data: +y = south, +x = east.  game location: +y = north, +x = east.
    //   The map_data polarity is consistent with the screen coordinate system for North = up.
    //   window_x[i] = (map_x[i] - min(map_x)) * window_size_x/map_size_x + window_x_offset
    //   window_x[i] = map_x[i] * scale_factor + offset where
    //     scale_factor = window_size_x/map_size_x and offset = window_x_offset - min(map_x)*scale_factor.
    const Vec2 screen_size = ZealService::get_instance()->dx->GetScreenRect();
    const float rect_left = map_rect_left * screen_size.x;
    const float rect_right = map_rect_right * screen_size.x;
    const float rect_top = map_rect_top * screen_size.y;
    const float rect_bottom = map_rect_bottom * screen_size.y;

    // TODO: Support zoom (scale and offset) in calculations below.
    float window_size_x = rect_right - rect_left;
    float window_size_y = rect_bottom - rect_top;
    float scale_factor_x = window_size_x / (zone_map_data->max_x - zone_map_data->min_x);
    float scale_factor_y = window_size_y / (zone_map_data->max_y - zone_map_data->min_y);
    scale_factor = min(scale_factor_x, scale_factor_y);  // Preserve aspect ratio.
    offset_x = rect_left - zone_map_data->min_x * scale_factor;
    offset_y = rect_top - zone_map_data->min_y * scale_factor;

    line_count = zone_map_data->num_lines;
    line_buffer_size = sizeof(MapVertex) * line_count * 2;
    std::unique_ptr<MapVertex[]> line_vertices = std::make_unique<MapVertex[]>(line_count * 2);

    // TODO: Cleanly clip (intersect the lines versus hard clamp when zoom enabled).
    for (int i = 0; i < line_count; ++i) {
        auto color = D3DCOLOR_XRGB(zone_map_data->lines[i].red, zone_map_data->lines[i].green, zone_map_data->lines[i].blue);
        if (color == D3DCOLOR_XRGB(0, 0, 0)) {
            color = D3DCOLOR_XRGB(128, 128, 128);
        }
        line_vertices[2 * i].x = max(rect_left, min(rect_right, zone_map_data->lines[i].x0 * scale_factor + offset_x));
        line_vertices[2 * i].y = max(rect_top, min(rect_bottom, zone_map_data->lines[i].y0 * scale_factor + offset_y));
        line_vertices[2 * i].z = 0.5f;
        line_vertices[2 * i].rhw = 1.f;
        line_vertices[2 * i].color = color;
        line_vertices[2 * i + 1].x = max(rect_left, min(rect_right, zone_map_data->lines[i].x1 * scale_factor + offset_x));
        line_vertices[2 * i + 1].y = max(rect_top, min(rect_bottom, zone_map_data->lines[i].y1 * scale_factor + offset_y));
        line_vertices[2 * i + 1].z = 0.5f;
        line_vertices[2 * i + 1].rhw = 1.f;
        line_vertices[2 * i + 1].color = color;
    }

    // Create a Vertex buffer and copy the map line segments over.
    if (FAILED(device->CreateVertexBuffer(line_buffer_size,
        D3DUSAGE_WRITEONLY,
        MapVertexFvfCode,
        D3DPOOL_MANAGED,
        &line_vertex_buffer))) {
        return;
    }

    BYTE* data = nullptr;
    if (FAILED(line_vertex_buffer->Lock(0, 0, &data, D3DLOCK_DISCARD))) {
        reset();  // Disable future attempts and release_resources().
        return;
    }
    memcpy(data, (const void*)line_vertices.get(), line_buffer_size);
    line_vertex_buffer->Unlock();

    // Create a worst-case sized Vertex buffer for live position updates.
    int position_buffer_size = sizeof(MapVertex) * kPositionVertices;
    if (FAILED(device->CreateVertexBuffer(position_buffer_size,
        D3DUSAGE_WRITEONLY,
        MapVertexFvfCode,
        D3DPOOL_MANAGED,
        &position_vertex_buffer))) {
        return;
    }

}

void ZoneMap::render_map()
{
    IDirect3DDevice8* device = ZealService::get_instance()->dx->GetDevice();
    if (!device || !line_count || !line_vertex_buffer)
        return;
     
    device->SetTexture(0, NULL);  // Ensure no texture is bound
    device->SetVertexShader(MapVertexFvfCode);
    device->SetStreamSource(0, line_vertex_buffer, sizeof(MapVertex));
    device->DrawPrimitive(D3DPT_LINELIST, 0, line_count);

    // Note: Markers are currently getting drawn under the map lines (good or bad?).
    if (position_vertex_buffer) {
        int triangle_count = render_update_position_buffer();
        device->SetStreamSource(0, position_vertex_buffer, sizeof(MapVertex));
        device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, triangle_count);
    }

    if (marker_vertex_buffer) {
        device->SetStreamSource(0, marker_vertex_buffer, sizeof(MapVertex));
        device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, kMarkerCount);
    }
}

int ZoneMap::render_update_position_buffer() {
    if (!position_vertex_buffer) {
        return 0;
    }

    // Translate loc (world) coordinates to screen coordinates. Note that the game
    // world coordinates are negated (polarity flipped) to match the map data.
    float screen_y = -position.x * scale_factor + offset_y;  // Note position is y,x,z.
    float screen_x = -position.y * scale_factor + offset_x;

    // Generate the vertices for two triangles that makeup the target.
    const Vec2 screen_size = ZealService::get_instance()->dx->GetScreenRect();
    float size = position_size * min(screen_size.x, screen_size.y);
    size = max(5.f, size);  // Constrain so it remains visible.

    // Heading: 0 = N = -y, 128 = W = -x, 256 = S = +y, 384 = E = +x.
    // So: Screen x tracks -sin(heading) and y tracks -cos(heading).
    float direction = static_cast<float>(heading * M_PI / 256); // In radians.
    const float rotation = static_cast<float>(120 * M_PI / 180);
    float cos_theta0 = cosf(direction);
    float sin_theta0 = sinf(direction);
    float cos_theta1 = cosf(direction - rotation);  // Rotated clockwise.
    float sin_theta1 = sinf(direction - rotation);
    float cos_theta2 = cosf(direction + rotation);  // Rotated CCW.
    float sin_theta2 = sinf(direction + rotation);

    Vec3 vertex0 = { -sin_theta0 * size, -cos_theta0 * size, 0 };
    Vec3 vertex1 = { -sin_theta1 * size, -cos_theta1 * size, 0 };
    Vec3 vertex2 = { -sin_theta2 * size, -cos_theta2 * size, 0 };
    Vec3 vertex3 = { -vertex0.x/8, -vertex0.y/8, 0 };

    Vec3 symbol[kPositionVertices] = { vertex0, vertex1, vertex3,
        vertex0, vertex3, vertex2};

    MapVertex position_vertices[kPositionVertices];
    const float rect_left = map_rect_left * screen_size.x;
    const float rect_right = map_rect_right * screen_size.x;
    const float rect_top = map_rect_top * screen_size.y;
    const float rect_bottom = map_rect_bottom * screen_size.y;
    for (int i = 0; i < kPositionVertices; ++i) {
        position_vertices[i].x = max(rect_left, min(rect_right, symbol[i].x + screen_x));
        position_vertices[i].y = max(rect_top, min(rect_bottom, symbol[i].y + screen_y));
        position_vertices[i].z = 0.5f;
        position_vertices[i].rhw = 1.f;
        position_vertices[i].color = D3DCOLOR_XRGB(0, 0, 255);
    }
    BYTE* data = nullptr;
    if (FAILED(position_vertex_buffer->Lock(0, 0, &data, D3DLOCK_DISCARD))) {
        position_vertex_buffer->Release();
        position_vertex_buffer = nullptr;
        return 0;
    }
    int position_buffer_size = sizeof(MapVertex) * kPositionVertices;
    memcpy(data, position_vertices, position_buffer_size);
    position_vertex_buffer->Unlock();

    return kPositionCount;
}

void ZoneMap::set_position_default_size(float new_size) {
    position_size = max(0.01f, min(0.1f, new_size));
    if (ZealService::get_instance() && ZealService::get_instance()->ini)
        ZealService::get_instance()->ini->setValue<float>("Zeal", "MapPositionSize", marker_size);
}

void ZoneMap::set_marker_default_size(float new_size) {
    marker_size = max(0.01f, min(0.1f, new_size));
    if (ZealService::get_instance() && ZealService::get_instance()->ini)
        ZealService::get_instance()->ini->setValue<float>("Zeal", "MapMarkerSize", marker_size);
}

void ZoneMap::clear_marker() {
    if (marker_vertex_buffer) {
        marker_vertex_buffer->Release();
        marker_vertex_buffer = nullptr;
    }
}

void ZoneMap::set_marker(int y, int x) {
    if (!line_vertex_buffer) {
        clear_marker();
        return;
    }

    IDirect3DDevice8* device = ZealService::get_instance()->dx->GetDevice();
    if (!device)
        return;

    // Translate loc (world) coordinates to screen coordinates. Note that the game
    // world coordinates are negated (polarity flipped) to match the map data.
    float screen_y = -y * scale_factor + offset_y;
    float screen_x = -x * scale_factor + offset_x;

    // Generate the vertices for four triangles that makeup the target.
    // Calculate the individual triangle 'size' in screen resolution.
    const Vec2 screen_size = ZealService::get_instance()->dx->GetScreenRect();
    float size = max(5.f, marker_size * min(screen_size.x, screen_size.y) * 0.5f);
    float short_size = size * 0.25f;
    const int kNumVertices = kMarkerCount * 3;  // Separate triangles with 3 vertices (CW direction).
    Vec3 marker[kNumVertices] = {{0, 0, 0}, {short_size, size, 0}, {-short_size, size, 0},
                                    {0, 0, 0}, {-short_size, -size, 0}, {short_size, -size, 0},
                                    {0, 0, 0}, {size, -short_size, 0}, {size, short_size, 0},
                                    {0, 0, 0}, {-size, short_size, 0}, {-size, -short_size, 0}
    };
    MapVertex marker_vertices[kNumVertices];
    const float rect_left = map_rect_left * screen_size.x;
    const float rect_right = map_rect_right * screen_size.x;
    const float rect_top = map_rect_top * screen_size.y;
    const float rect_bottom = map_rect_bottom * screen_size.y;
    for (int i = 0; i < kNumVertices; ++i) {
        marker_vertices[i].x = max(rect_left, min(rect_right, marker[i].x + screen_x));
        marker_vertices[i].y = max(rect_top, min(rect_bottom, marker[i].y + screen_y));
        marker_vertices[i].z = 0.5f;
        marker_vertices[i].rhw = 1.f;
        marker_vertices[i].color = D3DCOLOR_XRGB(255, 0, 0);
    }

    // Create a Vertex buffer and copy the triangle vertices.
    int marker_buffer_size = sizeof(MapVertex) * kNumVertices;
    if (FAILED(device->CreateVertexBuffer(marker_buffer_size,
        D3DUSAGE_WRITEONLY,
        MapVertexFvfCode,
        D3DPOOL_MANAGED,
        &marker_vertex_buffer))) {
        return;
    }

    BYTE* data = nullptr;
    if (FAILED(marker_vertex_buffer->Lock(0, 0, &data, D3DLOCK_DISCARD))) {
        marker_vertex_buffer->Release();
        marker_vertex_buffer = nullptr;
        return;
    }
    memcpy(data, marker_vertices, marker_buffer_size);
    marker_vertex_buffer->Unlock();
}

void ZoneMap::callback_render()
{
    if (!enabled || !Zeal::EqGame::is_in_game() || !Zeal::EqGame::get_self())
        return;

    Zeal::EqStructures::Entity* self = Zeal::EqGame::get_self();
    if (zone_id != self->ZoneId) {
        zone_id = self->ZoneId;
        load_map();
    }

    position = self->Position;
    heading = self->Heading;

    render_map();
}

void ZoneMap::set_enabled(bool _enabled, bool update_default)
{
    if (update_default) {
        if (!ZealService::get_instance() || !ZealService::get_instance()->ini)
            Zeal::EqGame::print_chat("Error trying to update ini.");
        else
            ZealService::get_instance()->ini->setValue<bool>("Zeal", "MapEnabled", _enabled);
    }
    enabled = _enabled;

    if (!enabled) {
        reset();  // Also releases resources.
    }
}

void ZoneMap::load_ini(IO_ini* ini)
{
    if (!ini->exists("Zeal", "MapEnabled"))
        ini->setValue<bool>("Zeal", "MapEnabled", false);
    if (!ini->exists("Zeal", "MapRectTop"))
        ini->setValue<float>("Zeal", "MapRectTop", kDefaultRectTop);
    if (!ini->exists("Zeal", "MapRectLeft"))
        ini->setValue<float>("Zeal", "MapRectLeft", kDefaultRectLeft);
    if (!ini->exists("Zeal", "MapRectBottom"))
        ini->setValue<float>("Zeal", "MapRectBottom", kDefaultRectBottom);
    if (!ini->exists("Zeal", "MapRectRight"))
        ini->setValue<float>("Zeal", "MapRectRight", kDefaultRectRight);
    if (!ini->exists("Zeal", "MapPositionSize"))
        ini->setValue<float>("Zeal", "MapPositionSize", kDefaultPositionSize);
    if (!ini->exists("Zeal", "MapMarkerSize"))
        ini->setValue<float>("Zeal", "MapMarkerSize", kDefaultMarkerSize);

    // TODO: Protect against corrupted ini file (like a boolean instead of float).
    enabled = ini->getValue<bool>("Zeal", "MapEnabled");
    map_rect_top = ini->getValue<float>("Zeal", "MapRectTop");
    map_rect_left = ini->getValue<float>("Zeal", "MapRectLeft");
    map_rect_bottom = ini->getValue<float>("Zeal", "MapRectBottom");
    map_rect_right = ini->getValue<float>("Zeal", "MapRectRight");
    position_size = ini->getValue<float>("Zeal", "MapPositionSize");
    marker_size = ini->getValue<float>("Zeal", "MapMarkerSize");

    // Sanity clamp ini values.
    if (map_rect_top < 0 || map_rect_top > map_rect_bottom || map_rect_bottom > 1) {
        map_rect_top = kDefaultRectTop;
        map_rect_bottom = kDefaultRectBottom;
    }
    if (map_rect_left < 0 || map_rect_left > map_rect_right || map_rect_right > 1) {
        map_rect_left = kDefaultRectLeft;
        map_rect_right = kDefaultRectRight;
    }
    position_size = max(0.01f, min(0.10f, position_size));
    marker_size = max(0.01f, min(0.10f, marker_size));
}

bool ZoneMap::set_map_rect(float top, float left, float bottom, float right) {
    const float kMinimumDimension = 0.1f;
    top = max(0.f, min(1.f, top));
    left = max(0.f, min(1.f, left));
    bottom = max(0.f, min(1.f, bottom));
    right = max(0.f, min(1.f, right));
    if ((top > bottom - kMinimumDimension) || (left > right - kMinimumDimension)) {
        Zeal::EqGame::print_chat("Invalid map dimensions");
        return false;  // Reject if not at least minimum size.
    }

    if (!ZealService::get_instance() || !ZealService::get_instance()->ini) {
        Zeal::EqGame::print_chat("Error trying to update ini.");
    }
    else {
        ZealService::get_instance()->ini->setValue<float>("Zeal", "MapRectTop", top);
        ZealService::get_instance()->ini->setValue<float>("Zeal", "MapRectLeft", left);
        ZealService::get_instance()->ini->setValue<float>("Zeal", "MapRectBottom", bottom);
        ZealService::get_instance()->ini->setValue<float>("Zeal", "MapRectRight", right);
    }

    //Zeal::EqGame::print_chat("Zone map rect: T:%f L:%f B:%f R:%f", top, left, bottom, right);
    map_rect_top = top;
    map_rect_left = left;
    map_rect_bottom = bottom;
    map_rect_right = right;

    release_resources();  // Invalidate buffers.
    zone_id = kInvalidZoneId;   // Triggers reload.  TODO: Preserve marker on a rect change.
    return true;
}

void ZoneMap::parse_rect(const std::vector<std::string>& args) {
    std::vector<float> tlbr;
    for (int i = 2; i < args.size(); i++)
    {
        float value = 0;
        if (Zeal::String::tryParse(args[i], &value))
            tlbr.push_back(value);
        else
            break;
    }

    if ((tlbr.size() != 4) || !set_map_rect(tlbr[0], tlbr[1], tlbr[2], tlbr[3]))
    {
        Zeal::EqGame::print_chat(USERCOLOR_SPELL_FAILURE, "Error: /map rect <top> <left> <bottom> <right> (fraction of screen dimensions)");
    }
}

void ZoneMap::parse_marker(const std::vector<std::string>& args) {
    if (args.size() <= 2) {
        clear_marker();
        return;
    }

    int y = 0;
    int x = 0;
    if (args.size() >= 4 && args.size() < 6 && Zeal::String::tryParse(args[2], &y) && Zeal::String::tryParse(args[3], &x)) {
        if (args.size() == 5) {
            float new_size = 0;
            if (!Zeal::String::tryParse(args[4], &new_size) || new_size <= 0) {
                clear_marker();
                return;
            }
            set_marker_default_size(new_size);
        }
        set_enabled(true);
        set_marker(y, x);
        return;
    }

    Zeal::EqGame::print_chat(USERCOLOR_SPELL_FAILURE, "Error: /map marker <y> <x> [size] (size: optional, 0 disables)");
}

bool ZoneMap::parse_shortcuts(const std::vector<std::string>& args) {
    // Support implicit shortcuts.
    int y = 0;
    int x = 0;
    if (args.size() == 3 && Zeal::String::tryParse(args[1], &y) && Zeal::String::tryParse(args[2], &x)) {
        set_enabled(true);
        set_marker(y, x);
        return true;
    }

    return false;
}

bool ZoneMap::parse_command(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        set_enabled(!enabled);
    } 
    else if (args[1] == "on") {
        set_enabled(true);
    }
    else if (args[1] == "off") {
        set_enabled(false);
    }
    else if (args[1] == "rect") {
        parse_rect(args);  // Set the Top, Left, Bottom, Right coordinates of window.
    }
    else if (args[1] == "marker") {
        parse_marker(args);
    }
    else if (!parse_shortcuts(args)) {
        Zeal::EqGame::print_chat("Usage: /map [on|off|rect|marker], /map <y> <x>");
    }
    return true;
}

ZoneMap::ZoneMap(ZealService* zeal, IO_ini* ini)
{
    load_ini(ini);
    zeal->callbacks->AddGeneric([this]() { callback_render(); }, callback_type::RenderUI);
    zeal->commands_hook->Add("/map", {}, "Controls map overlay",
        [this](const std::vector<std::string>& args) {
            return parse_command(args);
        });
}

ZoneMap::~ZoneMap()
{
    release_resources();
}