/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the game options class and related functions.
 */

#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED

#include <cstddef>
#include <map>
#include <vector>

#include "game_states/area_editor/editor.h"
#include "const.h"
#include "controls.h"
#include "utils/data_file.h"


using std::map;
using std::size_t;
using std::vector;


//Modes for the auto-throw feature.
enum AUTO_THROW_MODES {
    //Off.
    AUTO_THROW_OFF,
    //Hold button to auto-throw.
    AUTO_THROW_HOLD,
    //Press button to toggle auto-throw.
    AUTO_THROW_TOGGLE,
    
    //Total amount of auto-throw modes.
    N_AUTO_THROW_MODES,
};


/* ----------------------------------------------------------------------------
 * Game options.
 */
struct options_struct {

    static const float DEF_AREA_EDITOR_BACKUP_INTERVAL;
    static const float DEF_AREA_EDITOR_GRID_INTERVAL;
    static const bool DEF_AREA_EDITOR_SEL_TRANS;
    static const bool DEF_AREA_EDITOR_SHOW_EDGE_LENGTH;
    static const bool DEF_AREA_EDITOR_SHOW_TERRITORY;
    static const area_editor::SNAP_MODES DEF_AREA_EDITOR_SNAP_MODE;
    static const size_t DEF_AREA_EDITOR_SNAP_THRESHOLD;
    static const size_t DEF_AREA_EDITOR_UNDO_LIMIT;
    static const area_editor::VIEW_MODES DEF_AREA_EDITOR_VIEW_MODE;
    static const AUTO_THROW_MODES DEF_AUTO_THROW_MODE;
    static const float DEF_CURSOR_SPEED;
    static const bool DEF_DRAW_CURSOR_TRAIL;
    static const bool DEF_EDITOR_MMB_PAN;
    static const float DEF_EDITOR_MOUSE_DRAG_THRESHOLD;
    static const float DEF_EDITOR_PRIMARY_COLOR[3];
    static const float DEF_EDITOR_SECONDARY_COLOR[3];
    static const float DEF_EDITOR_TEXT_COLOR[3];
    static const bool DEF_EDITOR_SHOW_TOOLTIPS;
    static const float DEF_JOYSTICK_MIN_DEADZONE;
    static const float DEF_JOYSTICK_MAX_DEADZONE;
    static const size_t DEF_MAX_PARTICLES;
    static const bool DEF_MIPMAPS_ENABLED;
    static const bool DEF_MOUSE_MOVES_CURSOR[MAX_PLAYERS];
    static const bool DEF_SMOOTH_SCALING;
    static const bool DEF_SHOW_HUD_CONTROLS;
    static const unsigned int DEF_TARGET_FPS;
    static const bool DEF_TRUE_FULLSCREEN;
    static const bool DEF_WIN_FULLSCREEN;
    static const unsigned int DEF_WIN_H;
    static const bool DEF_WINDOW_POSITION_HACK;
    static const unsigned int DEF_WIN_W;
    static const float DEF_ZOOM_MID_LEVEL;
    
    //Backup the area in the area editor every X seconds.
    float area_editor_backup_interval;
    //Grid interval in the area editor, in units.
    float area_editor_grid_interval;
    //Can the user transform the selected vertexes?
    bool area_editor_sel_trans;
    //Show the length of an edge that's being drawn?
    bool area_editor_show_edge_length;
    //Show the selected mob(s)'s territory?
    bool area_editor_show_territory;
    //Snap mode to use.
    area_editor::SNAP_MODES area_editor_snap_mode;
    //Snap when the cursor is this close to a vertex/edge.
    size_t area_editor_snap_threshold;
    //Maximum number of undo operations.
    size_t area_editor_undo_limit;
    //View mode to use.
    area_editor::VIEW_MODES area_editor_view_mode;
    //Auto-throw mode.
    AUTO_THROW_MODES auto_throw_mode;
    //List of controls for each player.
    vector<vector<control_info> > controls;
    //Cursor speed, in pixels per second. N/A when using the mouse.
    float cursor_speed;
    //Draw a trail behind the mouse cursor?
    bool draw_cursor_trail;
    //If true, the middle mouse button pans in editors.
    bool editor_mmb_pan;
    //In editors, only consider a mouse drag if it moves these many pixels.
    float editor_mouse_drag_threshold;
    //List of which editor node widgets the user wants open.
    map<string, bool> editor_open_nodes;
    //Editor's custom style main reference color.
    ALLEGRO_COLOR editor_primary_color;
    //Editor's custom style accent reference color.
    ALLEGRO_COLOR editor_secondary_color;
    //In editors, show widget tooltips when the mouse is over them.
    bool editor_show_tooltips;
    //Editor's custom style text color.
    ALLEGRO_COLOR editor_text_color;
    //Should the editors use a custom style, or the default?
    bool editor_use_custom_style;
    //Player's intended option for fullscreen, before restarting the game.
    bool intended_win_fullscreen;
    //Player's intended option for window height, before restarting the game.
    int intended_win_h;
    //Player's intended option for window width, before restarting the game.
    int intended_win_w;
    //Maximum deadzone for joysticks.
    float joystick_max_deadzone;
    //Minimum deadzone for joysticks.
    float joystick_min_deadzone;
    //Maximum number of particles.
    size_t max_particles;
    //Enables or disables mipmaps.
    bool mipmaps_enabled;
    //For each player, does the mouse move their leader's cursor?
    bool mouse_moves_cursor[MAX_PLAYERS];
    //True to use interpolation when graphics are scaled up/down.
    bool smooth_scaling;
    //Show control icons on top of HUD elements?
    bool show_hud_controls;
    //Target framerate.
    int target_fps;
    //When using fullscreen, is this true fullscreen, or borderless window?
    bool true_fullscreen;
    //Should we force the window's positioning?
    //(on some machines it appears out-of-bounds by default)
    bool window_position_hack;
    //Set the camera's middle zoom level to this amount.
    float zoom_mid_level;
    
    void load(data_node* file);
    void save(data_node* file) const;
    
    options_struct();
    
private:

    void load_control(
        const BUTTONS action, const unsigned char player,
        const string &name, data_node* file, const string &def = ""
    );
    
};


#endif //ifndef OPTIONS_INCLUDED
