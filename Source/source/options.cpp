/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Game options class and related functions.
 */

#include <algorithm>

#include "options.h"

#include "functions.h"
#include "game.h"
#include "misc_structs.h"
#include "utils/string_utils.h"


//Default values for the different options.
const float options_struct::DEF_AREA_EDITOR_BACKUP_INTERVAL = 120.0f;
const float options_struct::DEF_AREA_EDITOR_GRID_INTERVAL = 32.0f;
const bool options_struct::DEF_AREA_EDITOR_SEL_TRANS = false;
const bool options_struct::DEF_AREA_EDITOR_SHOW_EDGE_LENGTH = true;
const bool options_struct::DEF_AREA_EDITOR_SHOW_TERRITORY = false;
const area_editor::SNAP_MODES options_struct::DEF_AREA_EDITOR_SNAP_MODE =
    area_editor::SNAP_GRID;
const size_t options_struct::DEF_AREA_EDITOR_SNAP_THRESHOLD = 80;
const size_t options_struct::DEF_AREA_EDITOR_UNDO_LIMIT = 20;
const area_editor::VIEW_MODES options_struct::DEF_AREA_EDITOR_VIEW_MODE =
    area_editor::VIEW_MODE_TEXTURES;
const AUTO_THROW_MODES options_struct::DEF_AUTO_THROW_MODE = AUTO_THROW_OFF;
const float options_struct::DEF_CURSOR_SPEED = 500.0f;
const bool options_struct::DEF_DRAW_CURSOR_TRAIL = true;
const bool options_struct::DEF_EDITOR_MMB_PAN = false;
const float options_struct::DEF_EDITOR_MOUSE_DRAG_THRESHOLD = 4;
const float options_struct::DEF_EDITOR_PRIMARY_COLOR[3] =
{0.05f, 0.05f, 0.05f};
const float options_struct::DEF_EDITOR_SECONDARY_COLOR[3] =
{0.19f, 0.47f, 0.78f};
const float options_struct::DEF_EDITOR_TEXT_COLOR[3] =
{1.0f, 1.0f, 1.0f};
const bool options_struct::DEF_EDITOR_SHOW_TOOLTIPS = true;
const float options_struct::DEF_JOYSTICK_MAX_DEADZONE = 0.9f;
const float options_struct::DEF_JOYSTICK_MIN_DEADZONE = 0.2f;
const size_t options_struct::DEF_MAX_PARTICLES = 200;
const bool options_struct::DEF_MIPMAPS_ENABLED = true;
const bool options_struct::DEF_MOUSE_MOVES_CURSOR[MAX_PLAYERS] =
{true, false, false, false};
const bool options_struct::DEF_SMOOTH_SCALING = true;
const bool options_struct::DEF_SHOW_HUD_CONTROLS = true;
const unsigned int options_struct::DEF_TARGET_FPS = 60;
const bool options_struct::DEF_TRUE_FULLSCREEN = false;
const bool options_struct::DEF_WIN_FULLSCREEN = false;
const unsigned int options_struct::DEF_WIN_H = 768;
const bool options_struct::DEF_WINDOW_POSITION_HACK = false;
const unsigned int options_struct::DEF_WIN_W = 1024;
const float options_struct::DEF_ZOOM_MID_LEVEL = 1.4f;


/* ----------------------------------------------------------------------------
 * Creates an options struct.
 */
options_struct::options_struct() :
    area_editor_backup_interval(DEF_AREA_EDITOR_BACKUP_INTERVAL),
    area_editor_grid_interval(DEF_AREA_EDITOR_GRID_INTERVAL),
    area_editor_sel_trans(DEF_AREA_EDITOR_SEL_TRANS),
    area_editor_show_edge_length(DEF_AREA_EDITOR_SHOW_EDGE_LENGTH),
    area_editor_show_territory(DEF_AREA_EDITOR_SHOW_TERRITORY),
    area_editor_snap_mode(DEF_AREA_EDITOR_SNAP_MODE),
    area_editor_snap_threshold(DEF_AREA_EDITOR_SNAP_THRESHOLD),
    area_editor_undo_limit(DEF_AREA_EDITOR_UNDO_LIMIT),
    area_editor_view_mode(DEF_AREA_EDITOR_VIEW_MODE),
    auto_throw_mode(DEF_AUTO_THROW_MODE),
    cursor_speed(DEF_CURSOR_SPEED),
    draw_cursor_trail(DEF_DRAW_CURSOR_TRAIL),
    editor_mmb_pan(DEF_EDITOR_MMB_PAN),
    editor_mouse_drag_threshold(DEF_EDITOR_MOUSE_DRAG_THRESHOLD),
    editor_show_tooltips(DEF_EDITOR_SHOW_TOOLTIPS),
    editor_use_custom_style(false),
    intended_win_fullscreen(DEF_WIN_FULLSCREEN),
    intended_win_h(DEF_WIN_H),
    intended_win_w(DEF_WIN_W),
    joystick_max_deadzone(DEF_JOYSTICK_MAX_DEADZONE),
    joystick_min_deadzone(DEF_JOYSTICK_MIN_DEADZONE),
    max_particles(DEF_MAX_PARTICLES),
    mipmaps_enabled(DEF_MIPMAPS_ENABLED),
    smooth_scaling(DEF_SMOOTH_SCALING),
    show_hud_controls(DEF_SHOW_HUD_CONTROLS),
    target_fps(DEF_TARGET_FPS),
    true_fullscreen(DEF_TRUE_FULLSCREEN),
    window_position_hack(DEF_WINDOW_POSITION_HACK),
    zoom_mid_level(DEF_ZOOM_MID_LEVEL) {
    
    mouse_moves_cursor[0] = DEF_MOUSE_MOVES_CURSOR[0];
    mouse_moves_cursor[1] = DEF_MOUSE_MOVES_CURSOR[1];
    mouse_moves_cursor[2] = DEF_MOUSE_MOVES_CURSOR[2];
    mouse_moves_cursor[3] = DEF_MOUSE_MOVES_CURSOR[3];
    
    editor_primary_color.r = DEF_EDITOR_PRIMARY_COLOR[0];
    editor_primary_color.g = DEF_EDITOR_PRIMARY_COLOR[1];
    editor_primary_color.b = DEF_EDITOR_PRIMARY_COLOR[2];
    editor_primary_color.a = 1.0f;
    
    editor_secondary_color.r = DEF_EDITOR_SECONDARY_COLOR[0];
    editor_secondary_color.g = DEF_EDITOR_SECONDARY_COLOR[1];
    editor_secondary_color.b = DEF_EDITOR_SECONDARY_COLOR[2];
    editor_secondary_color.a = 1.0f;
    
    editor_text_color.r = DEF_EDITOR_TEXT_COLOR[0];
    editor_text_color.g = DEF_EDITOR_TEXT_COLOR[1];
    editor_text_color.b = DEF_EDITOR_TEXT_COLOR[2];
    editor_text_color.a = 1.0f;
}


/* ----------------------------------------------------------------------------
 * Loads the player's options from a file.
 * file:
 *   File to read from.
 */
void options_struct::load(data_node* file) {
    reader_setter rs(file);
    
    /* Load controls. Format of a control:
     * "p<player>_<action>=<possible control 1>,<possible control 2>,<...>"
     * Format of a possible control:
     * "<input method>_<parameters, underscore separated>"
     * Input methods:
     * "k" (keyboard key), "mb" (mouse button),
     * "mwu" (mouse wheel up), "mwd" (down),
     * "mwl" (left), "mwr" (right), "jb" (joystick button),
     * "jap" (joystick axis, positive), "jan" (joystick axis, negative).
     * The parameters are the key/button number, joystick number,
     * joystick stick and axis, etc.
     * Check the constructor of control_info for more information.
     */
    for(unsigned char p = 0; p < MAX_PLAYERS; ++p) {
        controls[p].clear();
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            string internal_name = game.buttons.list[b].internal_name;
            if(internal_name.empty()) continue;
            load_control(game.buttons.list[b].id, p, internal_name, file);
        }
    }
    
    //Weed out controls that didn't parse correctly.
    for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
        for(size_t c = 0; c < controls[p].size(); ) {
            if(controls[p][c].action == BUTTON_NONE) {
                controls[p].erase(controls[p].begin() + c);
            } else {
                c++;
            }
        }
    }
    
    for(unsigned char p = 0; p < MAX_PLAYERS; ++p) {
        rs.set(
            "p" + i2s((p + 1)) + "_mouse_moves_cursor",
            mouse_moves_cursor[p]
        );
    }
    
    //Opened tree nodes in editors.
    editor_open_nodes.clear();
    vector<string> open_nodes_vector =
        semicolon_list_to_vector(
            file->get_child_by_name("editor_open_nodes")->value
        );
    for(size_t n = 0; n < open_nodes_vector.size(); ++n) {
        editor_open_nodes[open_nodes_vector[n]] = true;
    }
    
    //Other options.
    string resolution_str;
    unsigned char editor_snap_mode_c;
    unsigned char editor_view_mode_c;
    unsigned char auto_throw_mode_c;
    
    rs.set("area_editor_backup_interval", area_editor_backup_interval);
    rs.set("area_editor_grid_interval", area_editor_grid_interval);
    rs.set("area_editor_selection_transformation", area_editor_sel_trans);
    rs.set("area_editor_show_edge_length", area_editor_show_edge_length);
    rs.set("area_editor_show_territory", area_editor_show_territory);
    rs.set("area_editor_snap_mode", editor_snap_mode_c);
    rs.set("area_editor_snap_threshold", area_editor_snap_threshold);
    rs.set("area_editor_undo_limit", area_editor_undo_limit);
    rs.set("area_editor_view_mode", editor_view_mode_c);
    rs.set("auto_throw_mode", auto_throw_mode_c);
    rs.set("cursor_speed", cursor_speed);
    rs.set("draw_cursor_trail", draw_cursor_trail);
    rs.set("editor_mmb_pan", editor_mmb_pan);
    rs.set("editor_mouse_drag_threshold", editor_mouse_drag_threshold);
    rs.set("editor_primary_color", editor_primary_color);
    rs.set("editor_secondary_color", editor_secondary_color);
    rs.set("editor_show_tooltips", editor_show_tooltips);
    rs.set("editor_text_color", editor_text_color);
    rs.set("editor_use_custom_style", editor_use_custom_style);
    rs.set("fps", target_fps);
    rs.set("fullscreen", intended_win_fullscreen);
    rs.set("joystick_min_deadzone", joystick_min_deadzone);
    rs.set("joystick_max_deadzone", joystick_max_deadzone);
    rs.set("max_particles", max_particles);
    rs.set("middle_zoom_level", zoom_mid_level);
    rs.set("mipmaps", mipmaps_enabled);
    rs.set("resolution", resolution_str);
    rs.set("smooth_scaling", smooth_scaling);
    rs.set("show_hud_controls", show_hud_controls);
    rs.set("true_fullscreen", true_fullscreen);
    rs.set("window_position_hack", window_position_hack);
    
    auto_throw_mode =
        (AUTO_THROW_MODES)
        std::min(
            auto_throw_mode_c,
            (unsigned char) (N_AUTO_THROW_MODES - 1)
        );
    area_editor_snap_mode =
        (area_editor::SNAP_MODES)
        std::min(
            editor_snap_mode_c,
            (unsigned char) (area_editor::N_SNAP_MODES - 1)
        );
    area_editor_view_mode =
        (area_editor::VIEW_MODES)
        std::min(
            editor_view_mode_c,
            (unsigned char) (area_editor::N_VIEW_MODES - 1)
        );
    target_fps = std::max(1, target_fps);
    
    if(joystick_min_deadzone > joystick_max_deadzone) {
        std::swap(joystick_min_deadzone, joystick_max_deadzone);
    }
    if(joystick_min_deadzone == joystick_max_deadzone) {
        joystick_min_deadzone -= 0.1;
        joystick_max_deadzone += 0.1;
    }
    joystick_min_deadzone = clamp(joystick_min_deadzone, 0.0f, 1.0f);
    joystick_max_deadzone = clamp(joystick_max_deadzone, 0.0f, 1.0f);
    
    vector<string> resolution_parts = split(resolution_str);
    if(resolution_parts.size() >= 2) {
        intended_win_w = std::max(1, s2i(resolution_parts[0]));
        intended_win_h = std::max(1, s2i(resolution_parts[1]));
    }
    
    //Force the editor styles to be opaque, otherwise there can be problems.
    editor_primary_color.a = 1.0f;
    editor_secondary_color.a = 1.0f;
    editor_text_color.a = 1.0f;
}


/* ----------------------------------------------------------------------------
 * Loads a game control from the options file.
 * action:
 *   Load the control corresponding to this action.
 * player:
 *   Load the control corresponding to this player.
 * name:
 *   Name of the option in the file.
 * file:
 *   File to load from.
 * def:
 *   Default value of this control. Only applicable for player 1.
 */
void options_struct::load_control(
    const BUTTONS action, const unsigned char player,
    const string &name, data_node* file, const string &def
) {
    data_node* control_node =
        file->get_child_by_name("p" + i2s((player + 1)) + "_" + name);
    vector<string> possible_controls =
        semicolon_list_to_vector(
            control_node->get_value_or_default((player == 0) ? def : "")
        );
        
    for(size_t c = 0; c < possible_controls.size(); ++c) {
        controls[player].push_back(control_info(action, possible_controls[c]));
    }
}


/* ----------------------------------------------------------------------------
 * Saves the player's options into a file.
 * file:
 *   File to write to.
 */
void options_struct::save(data_node* file) const {
    //First, group the controls by action and player.
    map<string, string> grouped_controls;
    
    for(unsigned char p = 0; p < MAX_PLAYERS; ++p) {
        string prefix = "p" + i2s((p + 1)) + "_";
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            string internal_name = game.buttons.list[b].internal_name;
            if(internal_name.empty()) continue;
            grouped_controls[prefix + internal_name].clear();
        }
    }
    
    //Write down their control strings.
    for(size_t p = 0; p < MAX_PLAYERS; p++) {
        for(size_t c = 0; c < controls[p].size(); ++c) {
            string name = "p" + i2s(p + 1) + "_";
            
            for(size_t b = 0; b < N_BUTTONS; ++b) {
                if(game.buttons.list[b].internal_name.empty()) continue;
                if(controls[p][c].action == game.buttons.list[b].id) {
                    name += game.buttons.list[b].internal_name;
                    break;
                }
            }
            
            grouped_controls[name] += controls[p][c].stringify() + ";";
        }
    }
    
    //Save controls.
    for(auto &c : grouped_controls) {
        //Remove the final character, which is always an extra semicolon.
        if(c.second.size()) c.second.erase(c.second.size() - 1);
        
        file->add(new data_node(c.first, c.second));
    }
    
    for(unsigned char p = 0; p < MAX_PLAYERS; ++p) {
        file->add(
            new data_node(
                "p" + i2s((p + 1)) + "_mouse_moves_cursor",
                b2s(mouse_moves_cursor[p])
            )
        );
    }
    
    //Figure out the value for the editor tree node preferences.
    string open_nodes_str;
    for(auto n : editor_open_nodes) {
        if(n.second) {
            open_nodes_str += n.first + ";";
        }
    }
    if(!open_nodes_str.empty()) open_nodes_str.pop_back();
    
    //Other options.
    file->add(
        new data_node(
            "area_editor_backup_interval",
            f2s(area_editor_backup_interval)
        )
    );
    file->add(
        new data_node(
            "area_editor_grid_interval",
            i2s(area_editor_grid_interval)
        )
    );
    file->add(
        new data_node(
            "area_editor_selection_transformation",
            b2s(area_editor_sel_trans)
        )
    );
    file->add(
        new data_node(
            "area_editor_show_edge_length",
            b2s(area_editor_show_edge_length)
        )
    );
    file->add(
        new data_node(
            "area_editor_show_territory",
            b2s(area_editor_show_territory)
        )
    );
    file->add(
        new data_node(
            "area_editor_snap_mode",
            i2s(area_editor_snap_mode)
        )
    );
    file->add(
        new data_node(
            "area_editor_snap_threshold",
            i2s(area_editor_snap_threshold)
        )
    );
    file->add(
        new data_node(
            "area_editor_undo_limit",
            i2s(area_editor_undo_limit)
        )
    );
    file->add(
        new data_node(
            "area_editor_view_mode",
            i2s(area_editor_view_mode)
        )
    );
    file->add(
        new data_node(
            "auto_throw_mode",
            i2s(auto_throw_mode)
        )
    );
    file->add(
        new data_node(
            "cursor_speed",
            f2s(cursor_speed)
        )
    );
    file->add(
        new data_node(
            "draw_cursor_trail",
            b2s(draw_cursor_trail)
        )
    );
    file->add(
        new data_node(
            "editor_mmb_pan",
            b2s(editor_mmb_pan)
        )
    );
    file->add(
        new data_node(
            "editor_mouse_drag_threshold",
            i2s(editor_mouse_drag_threshold)
        )
    );
    file->add(
        new data_node(
            "editor_open_nodes",
            open_nodes_str
        )
    );
    file->add(
        new data_node(
            "editor_primary_color",
            c2s(editor_primary_color)
        )
    );
    file->add(
        new data_node(
            "editor_secondary_color",
            c2s(editor_secondary_color)
        )
    );
    file->add(
        new data_node(
            "editor_show_tooltips",
            b2s(editor_show_tooltips)
        )
    );
    file->add(
        new data_node(
            "editor_text_color",
            c2s(editor_text_color)
        )
    );
    file->add(
        new data_node(
            "editor_use_custom_style",
            b2s(editor_use_custom_style)
        )
    );
    file->add(
        new data_node(
            "fps",
            i2s(target_fps)
        )
    );
    file->add(
        new data_node(
            "fullscreen",
            b2s(intended_win_fullscreen)
        )
    );
    file->add(
        new data_node(
            "joystick_max_deadzone",
            f2s(joystick_max_deadzone)
        )
    );
    file->add(
        new data_node(
            "joystick_min_deadzone",
            f2s(joystick_min_deadzone)
        )
    );
    file->add(
        new data_node(
            "max_particles",
            i2s(max_particles)
        )
    );
    file->add(
        new data_node(
            "middle_zoom_level",
            f2s(zoom_mid_level)
        )
    );
    file->add(
        new data_node(
            "mipmaps",
            b2s(mipmaps_enabled)
        )
    );
    file->add(
        new data_node(
            "resolution",
            i2s(intended_win_w) + " " +
            i2s(intended_win_h)
        )
    );
    file->add(
        new data_node(
            "smooth_scaling",
            b2s(smooth_scaling)
        )
    );
    file->add(
        new data_node(
            "show_hud_controls",
            b2s(show_hud_controls)
        )
    );
    file->add(
        new data_node(
            "true_fullscreen",
            b2s(true_fullscreen)
        )
    );
    file->add(
        new data_node(
            "window_position_hack",
            b2s(window_position_hack)
        )
    );
}
