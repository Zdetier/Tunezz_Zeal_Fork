#pragma once
#include "hook_wrapper.h"
#include <stdint.h>
#include "EqStructures.h"
#include "EqUI.h"
#include "directx.h"
#include "zone_map_data.h"

class ZoneMap
{
public:
	struct AlignmentType { enum e: int{ kLeft = 0, kCenter, kRight, kFirst = kLeft, kLast = kRight }; };
	struct BackgroundType { enum e: int { kClear = 0, kDark, kLight, kTan, kFirst = kClear, kLast = kTan }; };
	struct LabelsMode { enum e : int { kOff = 0, kSummary, kAll, kFirst = kOff, kLast = kAll }; };

	static constexpr float kMaxPositionSize = 0.05f;  // In fraction of screen size.
	static constexpr float kMaxMarkerSize = 0.05f;

	ZoneMap(class ZealService* zeal, class IO_ini* ini);
	~ZoneMap();

	// UI control interface.
	// Rect and sizes are in percentages of screen dimensions (0 to 100).
	// Setting update_default stores to the ini. All trigger a ui_options update.
	bool is_enabled() const { return enabled; }
	void set_enabled(bool enable, bool update_default = false);
	bool set_background(int new_state, bool update_default = true); // [clear, dark, light, tan]
	bool set_background_alpha(int percent, bool update_default = true);
	bool set_alignment(int new_state, bool update_default = true); // [left, center, right]
	bool set_labels_mode(int new_mode, bool update_default = true);  // [off, summary, all]
	bool set_map_top(int top_percent, bool update_default = true, bool preserve_height = true);
	bool set_map_left(int left_percent, bool update_default = true, bool preserve_width = true);
	bool set_map_bottom(int bottom_percent, bool update_default = true);
	bool set_map_height(int height_percent, bool update_default = true);
	bool set_map_right(int right_percent, bool update_default = true);
	bool set_map_width(int width_percent, bool update_default = true);
	bool set_position_size(int new_size_percent, bool update_default = true);
	bool set_marker_size(int new_size_percent, bool update_default = true);
	bool set_zoom(int zoom_percent);  // Note: 100% = 1x.

	int get_background() const { return static_cast<int>(map_background_state); }
	int get_background_alpha() const { return static_cast<int>(map_background_alpha * 100 + 0.5f); }
	int get_alignment() const { return static_cast<int>(map_alignment_state); }
	int get_labels_mode() const { return static_cast<int>(map_labels_mode); }
	int get_map_top() const { return static_cast<int>(map_rect_top * 100 + 0.5f); }
	int get_map_left() const { return static_cast<int>(map_rect_left * 100 + 0.5f); }
	int get_map_bottom() const { return static_cast<int>(map_rect_bottom * 100 + 0.5f); }
	int get_map_height() const { return static_cast<int>((map_rect_bottom - map_rect_top) * 100 + 0.5f); }
	int get_map_right() const { return static_cast<int>(map_rect_right * 100 + 0.5f); }
	int get_map_width() const { return static_cast<int>((map_rect_right - map_rect_left) * 100 + 0.5f); }
	int get_position_size() const { return static_cast<int>(position_size / kMaxPositionSize * 100 + 0.5f); }
	int get_marker_size() const { return static_cast<int>(marker_size / kMaxMarkerSize * 100 + 0.5f); }
	int get_zoom() const { return static_cast<int>(zoom_factor * 100 + 0.5f); }

	void toggle_background();
	void toggle_zoom();
	void toggle_labels();
	void toggle_level_up();
	void toggle_level_down();

	// Adds a temporary "dynamic" text label to the current map with optional timeout that can be
	// used to mark locations or track player names. Note that y and x are in world (loc) coordinates.
	void add_dynamic_label(const std::string& label, int loc_y, int loc_x,
		unsigned int duration_ms = 0, D3DCOLOR font_color = D3DCOLOR_XRGB(250, 250, 51));

private:
	// DynamicLabels are added using add_label() and are in addition to the static map data labels.
	struct DynamicLabel {
		std::string label;  // Label to display.
		int loc_y = 0;  // Game location y.
		int loc_x = 0;  // Game location x.
		ULONGLONG timeout = 0;  // 0 disables the timeout.
		D3DCOLOR color = 0;
	};

	static constexpr int kInvalidZoneId = 0;
	static constexpr float kDefaultBackgroundAlpha = 0.5f;
	static constexpr float kDefaultRectTop = 0.1f;
	static constexpr float kDefaultRectLeft = 0.1f;
	static constexpr float kDefaultRectBottom = 0.4f;
	static constexpr float kDefaultRectRight = 0.4f;
	static constexpr float kDefaultPositionSize = 0.01f;
	static constexpr float kDefaultMarkerSize = 0.02f;

	// UI and parser methods.
	// Rect and sizes are in fractions of screen dimensions(0.f to 1.f).
	bool parse_command(const std::vector<std::string>& args);
	bool parse_shortcuts(const std::vector<std::string>& args);
	void parse_rect(const std::vector<std::string>& args);
	void parse_size(const std::vector<std::string>& args);
	void parse_alignment(const std::vector<std::string>& args);
	void parse_marker(const std::vector<std::string>& args);
	void parse_background(const std::vector<std::string>& args);
	void parse_zoom(const std::vector<std::string>& args);
	void parse_labels(const std::vector<std::string>& args);
	void parse_level(const std::vector<std::string>& args);
	void parse_poi(const std::vector<std::string>& args);
	bool search_poi(const std::string& search);
	void set_marker(int y, int x);
	void clear_marker();
	bool set_map_rect(float top, float left, float bottom, float right, bool update_default = false);
	bool set_level(int level);  // Set to 0 to show all levels.
	void update_ui_options();

	void load_ini(class IO_ini* ini);
	void save_ini();
	void dump();

	void callback_zone();
	void callback_render();

	// The following methods execute as part of callback_render().
	void render_release_resources();
	void render_load_map();
	void render_load_font();
	void render_load_labels(const ZoneMapData& zone_map_data);
	void render_map();
	void render_background();
	int render_update_position_buffer();
	void render_update_marker_buffer();
	void render_labels();
	void render_label_text(const char* label, int y, int x, D3DCOLOR font_color);

	bool enabled = false;
	BackgroundType::e map_background_state = BackgroundType::kClear;
	float map_background_alpha = kDefaultBackgroundAlpha;
	AlignmentType::e map_alignment_state = AlignmentType::kFirst;
	LabelsMode::e map_labels_mode = LabelsMode::kOff;
	int zone_id = kInvalidZoneId;
	int marker_zone_id = kInvalidZoneId;
	int zoom_recenter_zone_id = kInvalidZoneId;
	bool always_align_to_center = false;
	int marker_x = 0;
	int marker_y = 0;
	int map_level_zone_id = kInvalidZoneId;
	int map_level_index = 0;
	ZoneMapLabel map_level_label;
	char map_level_label_string[20];
	int dynamic_labels_zone_id = kInvalidZoneId;
	std::vector<DynamicLabel> dynamic_labels_list;  // Optional temporary labels.

	D3DVIEWPORT8 viewport = {};  // On-screen coordinates of viewport.
	float scale_factor = 0;  // Conversion factors for map data to screen coordinates.
	float zoom_factor = 1.f;
	float offset_x = 0;
	float offset_y = 0;
	float map_rect_top = kDefaultRectTop;
	float map_rect_left = kDefaultRectLeft;
	float map_rect_bottom = kDefaultRectBottom;
	float map_rect_right = kDefaultRectRight;
	float clip_rect_top = 0;
	float clip_rect_left = 0;
	float clip_rect_bottom = 0;
	float clip_rect_right = 0;
	int clip_max_z = 0;
	int clip_min_z = 0;
	float position_size = kDefaultPositionSize;
	float marker_size = kDefaultMarkerSize;

	std::vector<const ZoneMapLabel*> labels_list;  // List of pointers to visible map labels.
	int line_count = 0;  // # of primitives in line buffer.
	IDirect3DVertexBuffer8* line_vertex_buffer = nullptr;
	IDirect3DVertexBuffer8* position_vertex_buffer = nullptr;
	IDirect3DVertexBuffer8* marker_vertex_buffer = nullptr;
	ID3DXFont* label_font = nullptr;
};


