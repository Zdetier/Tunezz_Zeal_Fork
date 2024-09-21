### Description
Zeal adds quality of life functionality to the legacy (2002) Everquest client
used by most TAKP EqEMU players. The Miles Sound System used by that client
supports plug-ins for arbitrary filtering, and Zeal injects itself into the
client like a standard dll by using the .asi extension in the EQ root directory.
It can then patch itself into the client's processing loop and callbacks to
add new functionality and smooth out issues in the old client.

Zeal custom code is entirely open source and the releases are built directly
from the repo using github actions, providing clear transparency on the contents.

### Features
- Camera motion improvements
- Additional key binds (tab targeting, strafe, pet)
- Additional commands (melody, useitem, autoinventory)
- Additional ui support (new gauges, bag control, looting, spellsets, targetrings)
- Third party tool support (silent log messages, direct ZealPipes)
- Integrated map (see In-game Map section below)
- Various bug fixes
- Unique npc naming for better parsing

### Installation
A comprehensive guide, including troubleshooting, can be found at
https://quarm.guide/install-guides under "Installing Zeal" or a
simplified checklist under "Checklist for Installing Zeal".

Note that the method Zeal uses to inject itself into the client can trigger the
heuristics of anti-virus scanners to label it as malware. See trouble-shooting
guide above if your `Zeal.asi` file keeps disappearing.

A summary of the process is below:
1. Ensure that sound is enabled in the game (the volume level can be set to zero).
  - Check that `Sound=TRUE` in the eqclient.ini in your root Everquest directory.
2. [Download](https://github.com/iamclint/Zeal/releases) the latest Zeal
   release zip file (`zeal_v#.#.#_*.zip`) from the "Assets" section.
  - Ignore the Source code files. See troubleshooting if your browser complains.
3. Quit Everquest and extract the `Zeal.asi` from the zip file to your root
   Everquest install directory.
  - The `Zeal.pdb` file is a symbol file only needed by developers for debugging
    and can be ignored.
4. Extract the `EQUI_Options.xml` to the `uifiles/default` directory
   or to your active custom UI skin directory (like `uifiles/duxaui`).
  - Some functionality is only accessible through the updated options UI.
5. Recommended: Install a UI skin modified with the extra Zeal UI functionality
   such as mana gauges, experience per hour, loot all / link buttons, etc.
  - If the UI is not updated, there will be some XML error complaints that can be ignored
  - Check the list below, the Quarm discord `#ui-discussion` channel, or the Quarm guide
    link above for compatible UI's
6. Optional: Extract the `crashes` folder to your Everquest root directory.
  - If the folder is present and includes the ZealCrashSender.exe, a crash log
    can be automatically submitted for debugging.
7. Test Zeal installation in game by typing "/zeal version" and "/help zeal".
8. Configure Zeal using the new Zeal tabs in EQ options and assign new key binds.

### Compatible UI's
- https://github.com/NilliP/NillipussUI_1080p
- https://github.com/NilliP/NillipussUI_1440p
- https://www.eqinterface.com/downloads/fileinfo.php?id=6959
- https://github.com/LordDemonos/Quarm.Guide/blob/master/assets/duxaUI.7z?raw=true

### Chat % Replacements
- %n or %mana for mana%
- %h or %hp for hp%
- %loc for your location
- %th or %targethp for your targets health %

### Commands
___
- `/cc`
  - **Arguments:** ``
  - **Example:** `/cc`
  - **Description:** toggles classic classes in who and other areas.

- `/targetring`
  - **Arguments:** `size`, `indicator`
  - **Example:** `/targetring 0.25`
  - **Example:** `/targetring indicator` toggles auto attack indicator.
  - **Description:** toggles targetring on/off.

- `/resetexp`
  - **Arguments:** ``
  - **Example:** `/resetexp`
  - **Description:** resets the exp per hour calculations.
  - 
- `/timer`
  - **Arguments:** `int`
  - **Example:** `/timer 10`
  - **Description:** holds the last hotbutton pressed down for the duration (decisecond like /pause).
  
- `/pipedelay`
  - **Arguments:** `int`
  - **Example:** `/pipedelay 500`
  - **Description:** changes the delay between each loop of labels/gauges being sent out over the named pipe.

- `/pipe`
  - **Arguments:** `string`
  - **Example:** `/pipe set a respawn timer for 30 seconds`
  - **Description:** outputs a string through the named pipe.
  
- `/ttimer`
  - **Arguments:** `int`
  - **Example:** `/ttimer 500`
  - **Description:** change the delay in which a tooltip pops up on mouse hover.
  
- `/inspect target`
  - **Description:** adds target argument to /inspect, this just inspects your current target.

- `/loc noprint`
  - **Description:** adds noprint argument to /loc, this just sends loc directly to your log.

- `/reloadskin`
  - **Description:** reloads your current skin using ini.
    
- `/fov`
  - **Arguments:** `int`
  - **Example:** `/fov 65`
  - **Description:** changes your field of view with a value between 45 and 90.

- `/melody`
  - **Arguments:** `song gem #'s (maximum of 5)`
  - **Aliases:** `/mel`
  - **Example:** `/melody 1 4 2 3`
  - **Description:** plays songs in order until interrupted in any fashion.

- `/map`
  - **Arguments:** `on`, `off`, `size`, `alignment`, `marker`, `background`, `zoom`, `poi`, `labels`, `level`
  - **Example:** See In-game map section below
  - **Description:** controls map enable, size, labels, zoom, and markers
    
- `/pandelay`
  - **Arguments:** `ms delay`, `none`
  - **Example:** `/pandelay 200`
  - **Description:** changes the amount of delay before left click panning will start to happen
    
- `/hidecorpse`
  - **Arguments:** `looted`, `none`
  - **Aliases:** `/hideco`, `/hidec`, `/hc`
  - **Example:** `/hidecorpse looted`
  - **Description:** `looted` Hides a corpse after you have looted it., `none` reveals all hidden corpses

- `/spellset`
  - **Arguments:** `save`, `load`, `delete`
  - **Example:** `/spellset save buffs`
  - **Example:** `/spellset load buffs`
  - **Example:** `/spellset delete buffs`
  - **Description:** allows you to save and load spellsets

- `/showhelm`
  - **Aliases:** `/helm`
  - **Arguments:** `on, off`
  - **Example:** `/showhelm on`
  - **Description:** Toggles your helmet.

- `/showlootlockouts`
  - **Aliases:** `/showlootlockout`, `/showlockout`, `/sll`
  - **Description:** Shows you your current loot lockouts on Quarm.

- `/zealcam`
  - **Aliases:** `/smoothing`
  - **Arguments:** `x y 3rdperson_x 3rdperson_y`
  - **Example:** `/zealcam 0.7 0.2 0.7 0.2` if 3rd person arguments are not supplied, the first x and y are applied to both
  - **Description:** Toggles Zeal's mouse look smoothing methods, the first 2 arguments are first person sensitivity, and the last 2 are for 3rd person

- `/autoinventory`
  - **Aliases:** `/autoinv`, `/ai`
  - **Description:** Drops whatever is on your cursor into your inventory.

- `/autobank`
  - **Aliases:** `/autoba`, `/ab`
  - **Description:** Drops whatever is on your cursor into your bank. [requires you to be at a banker] (not fully functional atm)

- `/target`
  - **Aliases:** `/cleartarget`
  - **Description:** acts as normal /target unless you provide no argument in which case it will clear your target.

- `/sit`
  - **Description:** The /sit command now accepts "on" as an argument. Using "/sit on" will always make you sit, even if you are currently sitting. This matches the game's native "/sit off" which always makes you stand even if you are currently standing. The "/sit" command will continue to toggle sit/stand state if no argument is provided or if the argument provided is not on or off. Additionally, "/sit down" now works as well and will always make you sit, even if already sitting.

- `/camp`
  - **Description:** Auto sits before camping.

- `/zeal`
  - **Arguments:** `version`
  - **Description:** Shows the version of zeal.

- `/zealinput`
  - **Description:** toggles the zeal input setup for any input in game, giving you a more modern input (ctrl+c, ctrl+v, left, right, shift left+right for highlighting, home, end ect).

- `/help zeal`
  - **Description:** Shows the custom Zeal commands.

- `/timestamp`
  - **Aliases:** `/tms`
  - **Description:** Shows message timestamps.

- `/outputfile`
  - **Aliases:** `/output`, `/out`
  - **Arguments:** `inventory | spellbook` `[optional_name]`
  - **Example:** `/outputfile inventory my_inventory`
  - **Description:**
    - `inventory` outputs information about your equipment, inventory bag slots, held item, and bank slots to a file.
    - `spellbook` outputs a list of all spell ids current scribed in your spellbook.

- `/buffs`
  - **Description:** Outputs the players buff timers to the chat only if they are using OldUI.

- `/bluecon`
  - **Description:** Changes the blue con color to usercolor #70 which is otherwise unused, you can edit in the options window.

- `/alarm`
  - **Arguments:** `oldui`
  - **Description:** Re-opens the alarm window, if oldui is specified it allows for an alarm on it.
___
### Binds
- Cycle through nearest NPCs
- Cycle through nearest PCs
- Strafe Right
- Strafe Left
- Auto Inventory
- Toggle last 2 targets
- Reply target
- Pet Attack
- Pet Guard
- Pet Follow
- Pet Back
- Slow turn left
- Slow turn right
- Target nearest pc corpse
- Target nearest npc corpse
- Toggle map on/off
- Toggle through map default zooms
- Toggle through map backgrounds
- Toggle through map label modes
- Toggle up or down through visible map levels
___
### UI
- **Gauge EqType's**
  - `23` EXP Per Hour

- **Label EqType's**
  - `80` Mana/Max Mana
  - `81` Exp Per Hour Percentage
  - `124` Current Mana
  - `125` Max Mana
  - `134` Spell being casted

- **LootAllButton**
- **LinkAllButton**


### Options UI 
- **ScreenID Checkboxes**
  - Zeal_ShowHelm
  - Zeal_HideCorpse
  - Zeal_Cam
  - Zeal_BlueCon
  - Zeal_Timestamp
  - Zeal_Input
  - Zeal_Escape_
    
- **ScreenID Sliders**
  - Zeal_PanDelaySlider
  - Zeal_FirstPersonSlider_X
  - Zeal_FirstPersonSlider_Y
  - Zeal_ThirdPersonSlider_X
  - Zeal_ThirdPersonSlider_Y
    
- **ScreenID Labels**
  - Zeal_ThirdPersonLabel_X
  - Zeal_ThirdPersonLabel_Y
  - Zeal_FirstPersonLabel_X
  - Zeal_FirstPersonLabel_Y
  - Zeal_PanDelayLabel
___

### Zeal pipes in c#
- https://github.com/OkieDan/ZealPipes

### Building
#### Github official release builds
1. Commit an updated, unique ZEAL_VERSION in Zeal/Zeal.h that will be used as the release tag.
2. Go to the "Actions" tab of the Github workspace
3. Select the "Create Manual Release" workflow on the left
4. Click the drop-down menu on the right top side titled "Run workflow"
5. Select the branch with the commit to be released
6. Add a summary description that will be prepended to the change log notes
7. Click the green "Run workflow" button
8. After the green checkmark appears, go back to the main workspace and verify the content of the new release tag.

#### Local builds
Build in 32bit x86 mode using Microsoft Visual Studio 2022 (free Community edition works)


### In-game Map
#### Setup and configuration
Zeal 4.0 and later includes an integrated in-game map that contains the map data for
all zones through Planes of Power. The map is drawn into the game's DirectX viewport
as part of the rendering sequence and is currently not 'clickable'.

The map is controlled through three interfaces:
* Dedicated EQ options window tab (requires `EQUI_Options.xml`, see Installation notes above)
* Key binds for frequent map actions (configure in Options->Keyboard->UI)
* The /map command

The default map settings are stored in the EQClient.ini file of the root Everquest directory.
The defaults are updated when adjusting settings in the EQ options tab. The key binds and
/map commands are temporary changes unless the `/map save_ini` command is used.

It is recommended to use the Options tab to adjust the basic map settings to the preferred
defaults (size, position, background, marker sizes) and then use the keybinds for more
frequent map adjustments (on/off, toggle zoom, toggle backgrounds, toggle labels,
toggle visible levels).  The /map commands include extra options like poi search.

#### Enabling the map
* UI options checkbox
* Key bind: "Toggle Map" - Toggles map on and off
* Command examples:
  - `/map` - Toggles map on and off
  - `/map on` - Turns map on
  - `/map off` - Turns map off

#### Map size, position, and alignment
The map is drawn to fit within rectangular limits view defined by a top left corner,
a height, and a width. The zones have different aspect ratios, so some zones will scale
to fill the height and others the width.  The alignment setting controls where
the map goes when it is height constrained (top left, top center, top right).

* UI options sliders for top, left, height, and width and a combobox for alignment
* Command examples:
  - `/map size 2 3 50 60` map window top=2% left=3% height=50% width=60% of screen dimensions
  - `/map alignment center` aligns the aspect ratio constrained map to the top center of the window

#### Map background
The map supports four different options for the map background for contrast enhancement:
clear (0) , dark (1), light (2), or tan (3).  Additionally, it supports alpha transparency.

* UI options combo box for map background and slider for setting alpha as a percent
* Key bind: "Toggle Map Background" - toggles through the four settings
* Command examples:
  - `/map background 1` sets the background to dark with no change to alpha
  - `/map background 2 40` sets the background to light with 40% alpha

#### Map zoom
The default 100% map scale makes the entire zone visible sized to the height or width constraint.
In zoom, the map draws all available data that fits within the rectangular viewport. The 
view re-centers when the position marker is within 20% of an edge and moving towards that edge.

* UI options slider
* Key bind: "Toggle Map Zoom" - toggles through 100%, 200%, 400%, 800% zoom
* Command examples:
  - `/map zoom 200` sets map scaling to 200% (2x)

#### Showing group members
The map supports showing the live position of other group members. The group members are colored
in this order relative to their group listing: red, orange, green, blue, purple.

* UI options checkbox to enable / disable
* Command examples:
  - `/map show_group` toggles it on and off

#### Showing map levels
The map supports showing different levels based on the Brewall map color standards. Not all of
the zones are properly colored, but it does work well in some of the 3-D overlapping zones.

* Key bind: "Toggle Map Level Up", "Toggle Map Level Down" - toggles up or down the visible level
* Command examples:
  - `/map level` shows the current zone's map data level info
  - `/map level 0` shows default of all levels
  - `/map level 2` shows the current zone's level 2 data

#### Position marker
The map supports adding a position marker to easier identification of target coordinates.

* UI options slider to adjust the marker size
* Command examples:
  - `/map marker 1100 -500` sets a map marker at /loc of 1100, -500
  - `/map 1100 -500` is a shortcut for the command above to set a marker at 1100, -500
  - `/map marker` clears the marker
  - `/map 0` is a shortcut for clearing the marker

#### Map points of interest (poi), labels, and search
The map supports listing the available points of interest and adding them as labels to the map.

Note that DirectX rendering of text is slow and this is unoptimized, so using the all setting
for the labels can have a significant framerate impact (use keybind to toggle on and off).

* UI combobox for setting the labels mode (off, summary, all)
* Key bind: "Toggle Map Labels" - toggles through the labels modes
* Command examples:
  - `/map poi` lists the available poi's, including their indices
  - `/map poi 4` drops a marker on index [4] of the `/map poi` list
  - `/map poi butcherblock` performs a text search of the poi list for butcherblock, reports 
     any matches, and drops a marker on the first one
  - `/map butcherblock` shortcut for above (does not work for terms that match other commands)
  - `/map labels summary` enables the summary labels (other options are `off` or `all`)

#### Map data source
The map has simple support for external map data files. The map data_mode can be set to `internal`,
`both`, or `external`. In `both`, the internal maps are combined with any available data from an
external file for that zone. In `external`, the internal map data for the zone is ignored if
external file data exists for that zone. In all cases internal data is used if external data is
not present.

Note that some features, such as level recognition, are not currently supported with external data.

The external map files must be placed in a `map_files` directory in the root everquest directory
with zones named to match their short names (ie `map_files/commons.txt` contains the data for
West Commonlands).  An optional, `_1.txt` file (ie `map_files/commons_1.txt`) will also be
parsed if present, so Brewall map files with POIs can be directly dropped in.

The external map support requires a format compatible with Brewall map data.
```
L x0, y0, z0, x1, y1, z1, red, green, blue
P x, y, z, red, green, blue, ignored, label_string
```

* Command examples:
  - `/map data_mode both` adds external zone map file data if present to internal maps
  - `/map data_mode external` uses external zone map files if present to replace internal maps

