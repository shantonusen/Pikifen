/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally accessible functions.
 */

#define _USE_MATH_DEFINES

// Visual Studio warnings.
#ifdef _MSC_VER
// Disable warning about localtime being deprecated.
#pragma warning(disable : 4996)
#endif

#include <algorithm>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "functions.h"

#include "const.h"
#include "drawing.h"
#include "game.h"
#include "game_states/menus.h"
#include "init.h"
#include "utils/backtrace.h"
#include "utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Calls al_fwrite, but with an std::string instead of a c-string.
 * f:
 *   Allegro file.
 * s:
 *   String to write.
 */
void al_fwrite(ALLEGRO_FILE* f, string s) {
    al_fwrite(f, s.c_str(), s.size());
}


/* ----------------------------------------------------------------------------
 * Converts a color to its string representation.
 * c:
 *   Color to convert.
 */
string c2s(const ALLEGRO_COLOR &c) {
    return i2s(c.r * 255) + " " + i2s(c.g * 255) + " " + i2s(c.b * 255) +
           (c.a == 1 ? "" : " " + i2s(c.a * 255));
}


/* ----------------------------------------------------------------------------
 * Returns the color that was provided, but with the alpha changed.
 * c:
 *   The color to change the alpha on.
 * a:
 *   The new alpha, [0-255].
 */
ALLEGRO_COLOR change_alpha(const ALLEGRO_COLOR &c, const unsigned char a) {
    ALLEGRO_COLOR c2;
    c2.r = c.r; c2.g = c.g; c2.b = c.b;
    c2.a = a / 255.0;
    return c2;
}


/* ----------------------------------------------------------------------------
 * Returns the color provided, but darker or lighter by l amount.
 * c:
 *   The color to change the lighting on.
 * l:
 *   Lighting amount, positive or negative, from 0 to 1.
 */
ALLEGRO_COLOR change_color_lighting(const ALLEGRO_COLOR &c, const float l) {
    ALLEGRO_COLOR c2;
    c2.r = clamp(c.r + l, 0.0f, 1.0f);
    c2.g = clamp(c.g + l, 0.0f, 1.0f);
    c2.b = clamp(c.b + l, 0.0f, 1.0f);
    c2.a = c.a;
    return c2;
}


/* ----------------------------------------------------------------------------
 * Clears the textures of the area's sectors from memory.
 */
void clear_area_textures() {
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        if(
            s_ptr->texture_info.bitmap &&
            s_ptr->texture_info.bitmap != game.bmp_error
        ) {
            game.textures.detach(s_ptr->texture_info.file_name);
            s_ptr->texture_info.bitmap = NULL;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Purposely crashes the engine, reporting as much information as possible to
 * the logs. Used when a fatal problem occurs.
 * reason:
 *   Explanation of the type of crash (assert, SIGSEGV, etc.).
 * info:
 *   Any extra information to report to the logs.
 * exit_status:
 *   Program exit status.
 */
void crash(const string &reason, const string &info, const int exit_status) {

    if(game.display) {
        ALLEGRO_BITMAP* backbuffer = al_get_backbuffer(game.display);
        if(backbuffer) {
            al_save_bitmap(
                (
                    USER_DATA_FOLDER_PATH + "/" +
                    "Crash_" + get_current_time(true) + ".png"
                ).c_str(),
                backbuffer
            );
        }
    }
    
    string error_str = "Program crash!\n";
    error_str +=
        "  Reason: " + reason + ".\n"
        "  Info: " + info + "\n"
        "  Time: " + get_current_time(false) + ".\n";
    if(game.errors_reported_so_far > 0) {
        error_str += "  Error log has messages!\n";
    }
    error_str +=
        "  Game state: " + game.get_cur_state_name() + ". delta_t: " +
        (
            game.delta_t == 0.0f ? "0" :
            f2s(game.delta_t) + " (" + f2s(1.0f / game.delta_t) + " FPS)"
        ) + ".\n"
        "  Mob count: " +
        i2s(game.states.gameplay->mobs.all.size()) + ". Particle count: " +
        i2s(game.states.gameplay->particles.get_count()) + ".\n" +
        "  Bitmaps loaded: " + i2s(game.bitmaps.get_list_size()) + " (" +
        i2s(game.bitmaps.get_total_calls()) + " total calls).\n" +
        "  Current area: ";
        
    if(!game.cur_area_data.name.empty()) {
        error_str +=
            game.cur_area_data.name + ", version " +
            game.cur_area_data.version + ".\n";
    } else {
        error_str += "none.\n";
    }
    
    error_str += "  Current leader: ";
    
    if(game.states.gameplay->cur_leader_ptr) {
        error_str +=
            game.states.gameplay->cur_leader_ptr->type->name + ", at " +
            p2s(game.states.gameplay->cur_leader_ptr->pos) +
            ", state history: " +
            game.states.gameplay->cur_leader_ptr->fsm.cur_state->name;
        for(size_t h = 0; h < STATE_HISTORY_SIZE; ++h) {
            error_str +=
                " " +
                game.states.gameplay->cur_leader_ptr->
                fsm.prev_state_names[h];
        }
        error_str += "\n  10 closest Pikmin to that leader:\n";
        
        vector<pikmin*> closest_pikmin =
            game.states.gameplay->mobs.pikmin_list;
        sort(
            closest_pikmin.begin(), closest_pikmin.end(),
        [] (pikmin * p1, pikmin * p2) -> bool {
            return
            dist(
                game.states.gameplay->cur_leader_ptr->pos,
                p1->pos
            ).to_float() <
            dist(
                game.states.gameplay->cur_leader_ptr->pos,
                p2->pos
            ).to_float();
        }
        );
        
        size_t closest_p_amount = std::min(closest_pikmin.size(), (size_t) 10);
        for(size_t p = 0; p < closest_p_amount; ++p) {
            error_str +=
                "    " + closest_pikmin[p]->type->name + ", at " +
                p2s(closest_pikmin[p]->pos) + ", history: " +
                closest_pikmin[p]->fsm.cur_state->name;
            for(size_t h = 0; h < STATE_HISTORY_SIZE; ++h) {
                error_str += " " + closest_pikmin[p]->fsm.prev_state_names[h];
            }
            error_str += "\n";
        }
    } else {
        error_str += "none.";
    }
    
    log_error(error_str);
    
    show_message_box(
        NULL, "Program crash!",
        "Pikifen has crashed!",
        "Sorry about that! To help fix this problem, please read the "
        "FAQ & troubleshooting section of the included manual. Thanks!",
        NULL,
        ALLEGRO_MESSAGEBOX_ERROR
    );
    
    exit(exit_status);
}


/* ----------------------------------------------------------------------------
 * Checks whether a given edge should get a ledge smoothing edge offset effect
 * or not.
 * e_ptr:
 *   Edge to check.
 * affected_sector:
 *   If there should be an effect, this is the affected sector,
 *   i.e. the one getting the smoothing.
 * unaffected_sector:
 *   If there should be an effect, this is the unaffected sector,
 *   i.e. the lower one.
 */
bool does_edge_have_ledge_smoothing(
    edge* e_ptr, sector** affected_sector, sector** unaffected_sector
) {
    //Never-smooth walls don't have the effect.
    if(e_ptr->ledge_smoothing_length <= 0.0f) return false;
    
    if(
        (e_ptr->sectors[0] && !e_ptr->sectors[1]) ||
        e_ptr->sectors[1]->is_bottomless_pit
    ) {
        //If 1 exists but 0 doesn't.
        *affected_sector = e_ptr->sectors[0];
        *unaffected_sector = e_ptr->sectors[1];
        return true;
        
    } else if(
        (!e_ptr->sectors[0] && e_ptr->sectors[1]) ||
        e_ptr->sectors[0]->is_bottomless_pit
    ) {
        //If 1 exists but 0 doesn't.
        *affected_sector = e_ptr->sectors[1];
        *unaffected_sector = e_ptr->sectors[0];
        return true;
        
    } else {
        //Return whichever one is the tallest.
        if(e_ptr->sectors[0]->z > e_ptr->sectors[1]->z) {
            *affected_sector = e_ptr->sectors[0];
            *unaffected_sector = e_ptr->sectors[1];
            return true;
        } else if(e_ptr->sectors[1]->z > e_ptr->sectors[0]->z) {
            *affected_sector = e_ptr->sectors[1];
            *unaffected_sector = e_ptr->sectors[0];
            return true;
        } else {
            return false;
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Checks whether a given edge should get a liquid limit edge offset effect
 * or not.
 * e_ptr:
 *   Edge to check.
 * affected_sector:
 *   If there should be an effect, this is the affected sector,
 *   i.e. the one with the liquid.
 * unaffected_sector:
 *   If there should be an effect, this is the unaffected sector,
 *   i.e. the one without the liquid.
 */
bool does_edge_have_liquid_limit(
    edge* e_ptr, sector** affected_sector, sector** unaffected_sector
) {
    //Check if the sectors exist.
    if(!e_ptr->sectors[0] || !e_ptr->sectors[1]) return false;
    
    //Check which ones have liquid.
    bool has_liquid[2] = {false, false};
    for(unsigned char s = 0; s < 2; ++s) {
        for(size_t h = 0; h < e_ptr->sectors[s]->hazards.size(); ++h) {
            if(e_ptr->sectors[s]->hazards[h]->associated_liquid) {
                has_liquid[s] = true;
            }
        }
    }
    
    //Return edges with liquid on one side only.
    if(has_liquid[0] && !has_liquid[1]) {
        *affected_sector = e_ptr->sectors[0];
        *unaffected_sector = e_ptr->sectors[1];
        return true;
    } else if(has_liquid[1] && !has_liquid[0]) {
        *affected_sector = e_ptr->sectors[1];
        *unaffected_sector = e_ptr->sectors[0];
        return true;
    } else {
        return false;
    }
}


/* ----------------------------------------------------------------------------
 * Checks whether a given edge should get a wall shadow edge offset effect
 * or not.
 * e_ptr:
 *   Edge to check.
 * affected_sector:
 *   If there should be an effect, this is the affected sector,
 *   i.e. the one getting shaded.
 * unaffected_sector:
 *   If there should be an effect, this is the unaffected sector,
 *   i.e. the one casting the shadow.
 */
bool does_edge_have_wall_shadow(
    edge* e_ptr, sector** affected_sector, sector** unaffected_sector
) {
    //Never-cast walls don't cast.
    if(e_ptr->wall_shadow_length <= 0.0f) return false;
    
    //Invalid sectors don't cast.
    if(!e_ptr->sectors[0] || !e_ptr->sectors[1]) return false;
    if(e_ptr->sectors[0]->is_bottomless_pit) return false;
    if(e_ptr->sectors[1]->is_bottomless_pit) return false;
    
    //Same-height sectors can't cast.
    if(e_ptr->sectors[0]->z == e_ptr->sectors[1]->z) return false;
    
    //We can already save which one is highest.
    if(e_ptr->sectors[0]->z > e_ptr->sectors[1]->z) {
        *unaffected_sector = e_ptr->sectors[0];
        *affected_sector = e_ptr->sectors[1];
    } else {
        *unaffected_sector = e_ptr->sectors[1];
        *affected_sector = e_ptr->sectors[0];
    }
    
    if(e_ptr->wall_shadow_length != LARGE_FLOAT) {
        //Fixed shadow length.
        return true;
    } else {
        //Auto shadow length.
        return (*unaffected_sector)->z > (*affected_sector)->z + STEP_HEIGHT;
    }
}


/* ----------------------------------------------------------------------------
 * Stores the names of all files in a folder into a vector.
 * folder_name:
 *   Name of the folder.
 * folders:
 *   If true, only read folders. If false, only read files.
 * folder_found:
 *   If not NULL, returns whether the folder was found or not.
 */
vector<string> folder_to_vector(
    string folder_name, const bool folders, bool* folder_found
) {
    vector<string> v;
    
    if(folder_name.empty()) {
        if(folder_found) *folder_found = false;
        return v;
    }
    
    //Normalize the folder's path.
    folder_name = standardize_path(folder_name);
    
    ALLEGRO_FS_ENTRY* folder =
        al_create_fs_entry(folder_name.c_str());
    if(!folder || !al_open_directory(folder)) {
        if(folder_found) *folder_found = false;
        return v;
    }
    
    
    ALLEGRO_FS_ENTRY* entry = NULL;
    while((entry = al_read_directory(folder)) != NULL) {
        if(
            folders ==
            ((al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR) > 0)
        ) {
        
            string entry_name =
                standardize_path(al_get_fs_entry_name(entry));
                
            //Only save what's after the final slash.
            size_t pos = entry_name.find_last_of("/");
            if(pos != string::npos) {
                entry_name =
                    entry_name.substr(pos + 1, entry_name.size() - pos - 1);
            }
            
            v.push_back(entry_name);
        }
        al_destroy_fs_entry(entry);
    }
    al_close_directory(folder);
    al_destroy_fs_entry(folder);
    
    
    sort(v.begin(), v.end(), [] (string s1, string s2) -> bool {
        return str_to_lower(s1) < str_to_lower(s2);
    });
    
    if(folder_found) *folder_found = true;
    return v;
}


/* ----------------------------------------------------------------------------
 * Returns the blackout effect's strength
 * for the current time and weather.
 */
unsigned char get_blackout_strength() {
    size_t n_points =
        game.cur_area_data.weather_condition.blackout_strength.size();
        
    if(n_points == 0) {
        return 0;
    } else if(n_points == 1) {
        return game.cur_area_data.weather_condition.blackout_strength[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr =
            &game.cur_area_data.weather_condition.blackout_strength[p];
        auto next_ptr =
            &game.cur_area_data.weather_condition.blackout_strength[p + 1];
            
        if(
            game.states.gameplay->day_minutes >= cur_ptr->first &&
            game.states.gameplay->day_minutes < next_ptr->first
        ) {
        
            return
                interpolate_number(
                    game.states.gameplay->day_minutes,
                    cur_ptr->first, next_ptr->first,
                    cur_ptr->second, next_ptr->second
                );
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return 0;
}


/* ----------------------------------------------------------------------------
 * Returns the mob that is closest to the mouse cursor.
 */
mob* get_closest_mob_to_cursor() {
    dist closest_mob_to_cursor_dist = 0;
    mob* closest_mob_to_cursor = NULL;
    
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
        mob* m_ptr = game.states.gameplay->mobs.all[m];
        
        if(!m_ptr->fsm.cur_state) continue;
        if(m_ptr->stored_inside_another) continue;
        
        dist d = dist(game.mouse_cursor_w, m_ptr->pos);
        if(!closest_mob_to_cursor || d < closest_mob_to_cursor_dist) {
            closest_mob_to_cursor = m_ptr;
            closest_mob_to_cursor_dist = d;
        }
    }
    
    return closest_mob_to_cursor;
}


/* ----------------------------------------------------------------------------
 * Returns a string representing the current date and time.
 * filename_friendly:
 *   If true, slashes become dashes, and semicolons become dots.
 */
string get_current_time(const bool filename_friendly) {
    time_t tt;
    time(&tt);
    struct tm t = *localtime(&tt);
    return
        i2s(t.tm_year + 1900) +
        (filename_friendly ? "-" : "/") +
        leading_zero(t.tm_mon + 1) +
        (filename_friendly ? "-" : "/") +
        leading_zero(t.tm_mday) +
        (filename_friendly ? "_" : " ") +
        leading_zero(t.tm_hour) +
        (filename_friendly ? "." : ":") +
        leading_zero(t.tm_min) +
        (filename_friendly ? "." : ":") +
        leading_zero(t.tm_sec);
}


/* ----------------------------------------------------------------------------
 * Returns the daylight effect color for the current time and weather.
 */
ALLEGRO_COLOR get_daylight_color() {
    size_t n_points = game.cur_area_data.weather_condition.daylight.size();
    
    if(n_points == 0) {
        return al_map_rgba(255, 255, 255, 0);
    } else if(n_points == 1) {
        return game.cur_area_data.weather_condition.daylight[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr = &game.cur_area_data.weather_condition.daylight[p];
        auto next_ptr = &game.cur_area_data.weather_condition.daylight[p + 1];
        
        if(
            game.states.gameplay->day_minutes >= cur_ptr->first &&
            game.states.gameplay->day_minutes < next_ptr->first
        ) {
        
            return
                interpolate_color(
                    game.states.gameplay->day_minutes,
                    cur_ptr->first, next_ptr->first,
                    cur_ptr->second, next_ptr->second
                );
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return al_map_rgba(255, 255, 255, 0);
}


/* ----------------------------------------------------------------------------
 * Returns the fog color for the current time and weather.
 */
ALLEGRO_COLOR get_fog_color() {
    size_t n_points = game.cur_area_data.weather_condition.fog_color.size();
    
    if(n_points == 0) {
        return al_map_rgba(255, 255, 255, 0);
    } else if(n_points == 1) {
        return game.cur_area_data.weather_condition.fog_color[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr = &game.cur_area_data.weather_condition.fog_color[p];
        auto next_ptr = &game.cur_area_data.weather_condition.fog_color[p + 1];
        
        if(
            game.states.gameplay->day_minutes >= cur_ptr->first &&
            game.states.gameplay->day_minutes < next_ptr->first
        ) {
        
            return
                interpolate_color(
                    game.states.gameplay->day_minutes,
                    cur_ptr->first, next_ptr->first,
                    cur_ptr->second, next_ptr->second
                );
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return al_map_rgba(255, 255, 255, 0);
}


/* ----------------------------------------------------------------------------
 * Returns a name for the specified Allegro keyboard keycode.
 * This basically makes use of al_keycode_to_name, but with some special cases
 * and with some nice capitalization.
 * Returns an empty string on error.
 * keycode:
 *   Keycode to check.
 * condensed:
 *   If true, only the key name is returned. If false, some extra disambiguation
 *   is returned too, e.g. whether this is the left or right Ctrl.
 */
string get_key_name(const int keycode, const bool condensed) {
    switch(keycode) {
    case ALLEGRO_KEY_ESCAPE: {
        return "Esc";
    }
    case ALLEGRO_KEY_INSERT: {
        return "Ins";
    }
    case ALLEGRO_KEY_DELETE: {
        return "Del";
    }
    case ALLEGRO_KEY_PGUP: {
        return "PgUp";
    }
    case ALLEGRO_KEY_PGDN: {
        return "PgDn";
    }
    case ALLEGRO_KEY_PAD_0: {
        return "0 KP";
    }
    case ALLEGRO_KEY_PAD_1: {
        return "1 KP";
    }
    case ALLEGRO_KEY_PAD_2: {
        return "2 KP";
    }
    case ALLEGRO_KEY_PAD_3: {
        return "3 KP";
    }
    case ALLEGRO_KEY_PAD_4: {
        return "4 KP";
    }
    case ALLEGRO_KEY_PAD_5: {
        return "5 KP";
    }
    case ALLEGRO_KEY_PAD_6: {
        return "6 KP";
    }
    case ALLEGRO_KEY_PAD_7: {
        return "7 KP";
    }
    case ALLEGRO_KEY_PAD_8: {
        return "8 KP";
    }
    case ALLEGRO_KEY_PAD_9: {
        return "9 KP";
    }
    case ALLEGRO_KEY_PAD_ASTERISK: {
        return "* KP";
    }
    case ALLEGRO_KEY_PAD_DELETE: {
        return "Del KP";
    }
    case ALLEGRO_KEY_PAD_ENTER: {
        return "Enter KP";
    }
    case ALLEGRO_KEY_PAD_EQUALS: {
        return "= KP";
    }
    case ALLEGRO_KEY_PAD_MINUS: {
        return "- KP";
    }
    case ALLEGRO_KEY_PAD_PLUS: {
        return "+ KP";
    }
    case ALLEGRO_KEY_PAD_SLASH: {
        return "/ KP";
    }
    case ALLEGRO_KEY_LSHIFT: {
        if(!condensed) {
            return "Shift (left)";
        } else {
            return "Shift";
        }
    }
    case ALLEGRO_KEY_RSHIFT: {
        if(!condensed) {
            return "Shift (right)";
        } else {
            return "Shift";
        }
    }
    case ALLEGRO_KEY_ALT: {
        return "Alt";
    }
    case ALLEGRO_KEY_ALTGR: {
        return "AltGr";
    }
    case ALLEGRO_KEY_LCTRL: {
        if(!condensed) {
            return "Ctrl (left)";
        } else {
            return "Ctrl";
        }
    }
    case ALLEGRO_KEY_RCTRL: {
        if(!condensed) {
            return "Ctrl (right)";
        } else {
            return "Ctrl";
        }
    }
    case ALLEGRO_KEY_BACKSLASH:
    case ALLEGRO_KEY_BACKSLASH2: {
        return "\\";
    }
    case ALLEGRO_KEY_BACKSPACE: {
        if(!condensed) {
            return "Backspace";
        } else {
            return "BkSpc";
        }
    }
    case ALLEGRO_KEY_ENTER: {
        return "Enter";
    }
    }
    string name = str_to_title(al_keycode_to_name(keycode));
    for(size_t c = 0; c < name.size(); ++c) {
        if(name[c] == '_') name[c] = ' ';
    }
    return name;
}


/* ----------------------------------------------------------------------------
 * Returns the color a ledge's smoothing should be.
 * e_ptr:
 *   Edge with the ledge.
 */
ALLEGRO_COLOR get_ledge_smoothing_color(edge* e_ptr) {
    return e_ptr->ledge_smoothing_color;
}


/* ----------------------------------------------------------------------------
 * Returns the length a ledge's smoothing should be.
 * e_ptr:
 *   Edge with the ledge.
 */
float get_ledge_smoothing_length(edge* e_ptr) {
    return e_ptr->ledge_smoothing_length;
}


/* ----------------------------------------------------------------------------
 * Returns the color a liquid limit's effect should be.
 * e_ptr:
 *   Edge with the liquid limit.
 */
ALLEGRO_COLOR get_liquid_limit_color(edge* e_ptr) {
    return {1.0f, 1.0f, 1.0f, 0.75f};
}


/* ----------------------------------------------------------------------------
 * Returns the length a liquid's limit effect.
 * e_ptr:
 *   Edge with the liquid limit.
 */
float get_liquid_limit_length(edge* e_ptr) {
    //Let's vary the length randomly by the topleftmost edge coordinates.
    //It's better to use this than using just the first edge, for instance,
    //because that would result in many cases of edges that share a first
    //vertex. So it wouldn't look as random.
    //It is much more rare for two edges to share a topleftmost vertex.
    float min_x = std::min(e_ptr->vertexes[0]->x, e_ptr->vertexes[1]->x);
    float min_y = std::min(e_ptr->vertexes[0]->y, e_ptr->vertexes[1]->y);
    float r = (hash_nr2(min_x, min_y) / (float) UINT32_MAX) * 5.0f;
    return
        15.0f +
        12.0f * sin((game.states.gameplay->area_time_passed * 2.0f) + r);
}


/* ----------------------------------------------------------------------------
 * Returns the width and height of a block of multi-line text.
 * Lines are split by a single "\n" character.
 * These are the dimensions of a bitmap
 * that would hold a drawing by draw_text_lines().
 * font:
 *   The text's font.
 * text:
 *   The text.
 * ret_w:
 *   The width gets returned here, if not NULL.
 * ret_h:
 *   The height gets returned here, if not NULL.
 */
void get_multiline_text_dimensions(
    const ALLEGRO_FONT* const font, const string &text, int* ret_w, int* ret_h
) {
    vector<string> lines = split(text, "\n", true);
    int fh = al_get_font_line_height(font);
    size_t n_lines = lines.size();
    
    if(ret_h) *ret_h = std::max(0, (int) ((fh + 1) * n_lines) - 1);
    
    if(ret_w) {
        int largest_w = 0;
        for(size_t l = 0; l < lines.size(); ++l) {
            largest_w =
                std::max(
                    largest_w, al_get_text_width(font, lines[l].c_str())
                );
        }
        
        *ret_w = largest_w;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the sun strength for the current time and weather.
 */
float get_sun_strength() {
    size_t n_points = game.cur_area_data.weather_condition.sun_strength.size();
    
    if(n_points == 0) {
        return 1.0f;
    } else if(n_points == 1) {
        return game.cur_area_data.weather_condition.sun_strength[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr =
            &game.cur_area_data.weather_condition.sun_strength[p];
        auto next_ptr =
            &game.cur_area_data.weather_condition.sun_strength[p + 1];
            
        if(
            game.states.gameplay->day_minutes >= cur_ptr->first &&
            game.states.gameplay->day_minutes < next_ptr->first
        ) {
        
            return
                interpolate_number(
                    game.states.gameplay->day_minutes,
                    cur_ptr->first, next_ptr->first,
                    cur_ptr->second, next_ptr->second
                ) / 255.0f;
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return 1.0f;
}


/* ----------------------------------------------------------------------------
 * Calculates the vertex info necessary to draw the throw preview line,
 * from a given start point to a given end point.
 * The vertexes returned always come in groups of four, and each group
 * must be drawn individually with the ALLEGRO_PRIM_TRIANGLE_FAN type.
 * Returns the amount of vertexes needed.
 * vertexes:
 *   The array of vertexes to fill. Must have room for at least 16.
 * start:
 *   Start the line at this point. This is a ratio from the leader (0) to
 *   the cursor (1).
 * end:
 *   Same as start, but for the end point.
 * leader_pos:
 *   Position of the leader.
 * cursor_pos:
 *   Position of the cursor.
 * color:
 *   Color of the line.
 * u_offset:
 *   Offset the texture u by this much.
 * u_scale:
 *   Scale the texture u by this much.
 * vary_thickness:
 *   If true, thickness varies as the line goes forward. False makes it use the
 *   same thickness (the minimal one) throughout.
 */
unsigned char get_throw_preview_vertexes(
    ALLEGRO_VERTEX* vertexes,
    const float start, const float end,
    const point &leader_pos, const point &cursor_pos,
    const ALLEGRO_COLOR &color,
    const float u_offset, const float u_scale,
    const bool vary_thickness
) {
    const float FADE_IN_RATIO = 0.30f;
    const float FADE_OUT_RATIO = 1.0f - FADE_IN_RATIO;
    const float MIN_THICKNESS = 2.0f;
    const float DEF_MAX_THICKNESS = 8.0f;
    const float segment_points[] = {
        0.0f, FADE_IN_RATIO, 0.5f, FADE_OUT_RATIO, 1.0f
    };
    
    float max_thickness = vary_thickness ? DEF_MAX_THICKNESS : MIN_THICKNESS;
    
    float leader_to_cursor_dist = dist(leader_pos, cursor_pos).to_float();
    unsigned char cur_v = 0;
    
    auto get_thickness =
        [MIN_THICKNESS, max_thickness]
    (float n) -> float {
        if(n >= 0.5f) {
            n = 1 - n;
        }
        return
        interpolate_number(
            n, 0.0f, 0.5f, MIN_THICKNESS, max_thickness
        );
    };
    auto get_color =
        [&color, FADE_IN_RATIO]
    (float n) -> ALLEGRO_COLOR {
        if(n >= 0.5f) {
            n = 1 - n;
        }
        if(n < FADE_IN_RATIO) {
            return
            interpolate_color(
                n, 0.0f, FADE_IN_RATIO,
                change_alpha(color, 0),
                color
            );
        } else {
            return color;
        }
    };
    
    //Get the vertexes of each necessary segment.
    for(unsigned char segment = 0; segment < 4; ++segment) {
        float segment_start = std::max(segment_points[segment], start);
        float segment_end = std::min(segment_points[segment + 1], end);
        
        if(
            segment_start > segment_points[segment + 1] ||
            segment_end < segment_points[segment]
        ) {
            continue;
        }
        
        vertexes[cur_v].x = leader_to_cursor_dist * segment_start;
        vertexes[cur_v].y = -get_thickness(segment_start) / 2.0f;
        vertexes[cur_v].color = get_color(segment_start);
        cur_v++;
        
        vertexes[cur_v] = vertexes[cur_v - 1];
        vertexes[cur_v].y = -vertexes[cur_v].y;
        cur_v++;
        
        vertexes[cur_v].x = leader_to_cursor_dist * segment_end;
        vertexes[cur_v].y = get_thickness(segment_end) / 2.0f;
        vertexes[cur_v].color = get_color(segment_end);
        cur_v++;
        
        vertexes[cur_v] = vertexes[cur_v - 1];
        vertexes[cur_v].y = -vertexes[cur_v].y;
        cur_v++;
    }
    
    //Final setup on all points.
    for(unsigned char v = 0; v < cur_v; ++v) {
        point p(vertexes[v].x, vertexes[v].y);
        
        //Apply the texture UVs.
        vertexes[v].u = vertexes[v].x / u_scale - u_offset;
        vertexes[v].v = vertexes[v].y;
        
        //Rotate and move all points. For the sake of simplicity, up until now,
        //they were assuming the throw is perfectly to the right (0 degrees),
        //and that it starts on the world origin.
        p = rotate_point(p, get_angle(leader_pos, cursor_pos));
        p += leader_pos;
        vertexes[v].x = p.x;
        vertexes[v].y = p.y;
        
        //Give Z a value.
        vertexes[v].z = 0.0f;
    }
    
    return cur_v;
}


/* ----------------------------------------------------------------------------
 * Given a string representation of mob script variables,
 * returns a map, where every key is a variable, and every value is the
 * variable's value.
 * vars_string:
 *   String with the variables.
 */
map<string, string> get_var_map(const string &vars_string) {
    map<string, string> final_map;
    vector<string> raw_vars = semicolon_list_to_vector(vars_string);
    
    for(size_t v = 0; v < raw_vars.size(); ++v) {
        vector<string> raw_parts = split(raw_vars[v], "=");
        if(raw_parts.size() < 2) {
            continue;
        }
        final_map[trim_spaces(raw_parts[0])] = trim_spaces(raw_parts[1]);
    }
    return final_map;
}


/* ----------------------------------------------------------------------------
 * Returns the color a wall's shadow should be.
 * e_ptr:
 *   Edge with the wall.
 */
ALLEGRO_COLOR get_wall_shadow_color(edge* e_ptr) {
    return e_ptr->wall_shadow_color;
}


/* ----------------------------------------------------------------------------
 * Returns the length a wall's shadow should be.
 * e_ptr:
 *   Edge with the wall.
 */
float get_wall_shadow_length(edge* e_ptr) {
    if(e_ptr->wall_shadow_length != LARGE_FLOAT) {
        return e_ptr->wall_shadow_length;
    }
    
    float height_difference =
        fabs(e_ptr->sectors[0]->z - e_ptr->sectors[1]->z);
    return
        clamp(
            height_difference * edge::SHADOW_AUTO_LENGTH_MULT,
            edge::SHADOW_MIN_AUTO_LENGTH,
            edge::SHADOW_MAX_AUTO_LENGTH
        );
}


/* ----------------------------------------------------------------------------
 * Auxiliary function that returns a table used in the weather configs.
 * node:
 *   Data node with the weather table.
 */
vector<std::pair<size_t, string> > get_weather_table(data_node* node) {
    vector<std::pair<size_t, string> > table;
    size_t n_points = node->get_nr_of_children();
    
    bool have_midnight = false;
    
    for(size_t p = 0; p < n_points; ++p) {
        data_node* point_node = node->get_child(p);
        
        size_t point_time = s2i(point_node->name);
        string point_value = point_node->value;
        
        table.push_back(make_pair(point_time, point_value));
        
        if(point_time == 24 * 60) have_midnight = true;
    }
    
    sort(
        table.begin(), table.end(),
        [] (
            std::pair<size_t, string> p1,
            std::pair<size_t, string> p2
    ) -> bool {
        return p1.first < p2.first;
    }
    );
    
    if(!table.empty()) {
        if(!have_midnight) {
            //If there is no data for the last hour,
            //use the data from the first point
            //(this is because the day loops after 24:00;
            //needed for interpolation).
            table.push_back(
                make_pair(24 * 60, table[0].second)
            );
        }
    }
    
    return table;
}


/* ----------------------------------------------------------------------------
 * Returns the interpolation between two colors, given a number in an interval.
 * input:
 *   The input number.
 * input_start:
 *   Start of the interval the input number falls on, inclusive.
 *   The closer to input_start, the closer the output is to output_start.
 * input_end:
 *   End of the interval the number falls on, inclusive.
 * output_start:
 *   Color on the starting tip of the interpolation.
 * output_end:
 *   Color on the ending tip of the interpolation.
 */
ALLEGRO_COLOR interpolate_color(
    const float input, const float input_start, const float input_end,
    const ALLEGRO_COLOR &output_start, const ALLEGRO_COLOR &output_end
) {
    float progress =
        (float) (input - input_start) / (float) (input_end - input_start);
    return
        al_map_rgba_f(
            output_start.r + progress * (output_end.r - output_start.r),
            output_start.g + progress * (output_end.g - output_start.g),
            output_start.b + progress * (output_end.b - output_start.b),
            output_start.a + progress * (output_end.a - output_start.a)
        );
}


/* ----------------------------------------------------------------------------
 * Prints something onto the error log.
 * s:
 *   String that represents the error.
 * d:
 *   If not null, this will be used to obtain the file name
 *   and line that caused the error.
 */
void log_error(string s, data_node* d) {
    if(d) {
        s += " (" + d->file_name;
        if(d->line_nr != 0) s += " line " + i2s(d->line_nr);
        s += ")";
    }
    s += "\n";
    
    std::cout << s;
    
    if(game.errors_reported_so_far == 0) {
        s =
            "\n" +
            get_current_time(false) +
            "; Pikifen version " +
            i2s(VERSION_MAJOR) + "." + i2s(VERSION_MINOR) +
            "." + i2s(VERSION_REV);
        if(!game.config.version.empty()) {
            s +=
                ", game version " +
                game.config.version + "\n" + s;
        }
    }
    
    string prev_error_log;
    string line;
    ALLEGRO_FILE* file_i = al_fopen(ERROR_LOG_FILE_PATH.c_str(), "r");
    if(file_i) {
        while(!al_feof(file_i)) {
            getline(file_i, line);
            prev_error_log += line + "\n";
        }
        prev_error_log.erase(prev_error_log.size() - 1);
        al_fclose(file_i);
    }
    
    ALLEGRO_FILE* file_o = al_fopen(ERROR_LOG_FILE_PATH.c_str(), "w");
    if(file_o) {
        al_fwrite(file_o, prev_error_log + s);
        al_fclose(file_o);
    }
    
    game.errors_reported_so_far++;
}


/* ----------------------------------------------------------------------------
 * Converts a point to a string.
 * p:
 *   Point to convert.
 * z:
 *   If not NULL, the third word is placed here.
 */
string p2s(const point &p, float* z) {
    return f2s(p.x) + " " + f2s(p.y) + (z ? " " + f2s(*z) : "");
}


/* ----------------------------------------------------------------------------
 * Prints a bit of info onto the screen, for some seconds.
 * text:
 *   Text to print. Can use line breaks.
 * total_duration:
 *   Total amount of time in which the text is present.
 * fade_duration:
 *   When closing, fade out in the last N seconds.
 */
void print_info(
    const string &text, const float total_duration, const float fade_duration
) {
    game.maker_tools.info_print_text = text;
    game.maker_tools.info_print_duration = total_duration;
    game.maker_tools.info_print_fade_duration = fade_duration;
    game.maker_tools.info_print_timer.start(total_duration);
}


/* ----------------------------------------------------------------------------
 * Creates and opens an Allegro native file dialog, and returns
 * the user's choice(s).
 * initial_path:
 *   Initial path for the dialog.
 * title:
 *   Title of the dialog.
 * patterns:
 *   File name patterns to match, separated by semicolon.
 * mode:
 *   al_create_native_file_dialog mode flags.
 */
vector<string> prompt_file_dialog(
    const string &initial_path, const string &title,
    const string &patterns, const int mode
) {
    ALLEGRO_FILECHOOSER* dialog =
        al_create_native_file_dialog(
            initial_path.c_str(), title.c_str(),
            patterns.c_str(), mode
        );
    al_show_native_file_dialog(game.display, dialog);
    
    //Reset the locale, which gets set by Allegro's native dialogs...
    //and breaks s2f().
    setlocale(LC_ALL, "C");
    
    vector<string> result;
    size_t n_choices = al_get_native_file_dialog_count(dialog);
    for(size_t c = 0; c < n_choices; ++c) {
        result.push_back(
            standardize_path(
                al_get_native_file_dialog_path(dialog, c)
            )
        );
    }
    
    al_destroy_native_file_dialog(dialog);
    return result;
}


/* ----------------------------------------------------------------------------
 * Creates and opens an Allegro native file dialog, and returns
 * the user's choice(s), but confines the results to a specific folder.
 * The result pointer returns FILE_DIALOG_RES_SUCCESS on success,
 * FILE_DIALOG_RES_WRONG_FOLDER if the one or more choices do not belong to
 * the specified folder, and FILE_DIALOG_RES_CANCELED if the user canceled.
 * The list of choices that are returned only have the file name, not the
 * rest of the path. Choices can also be contained inside subfolders of the
 * specified folder.
 * folder:
 *   The folder to lock to, without the ending slash.
 * title:
 *   Title of the dialog.
 * patterns:
 *   File name patterns to match, separated by semicolon.
 * mode:
 *   al_create_native_file_dialog mode flags.
 * result:
 *   The result of the dialog is returned here.
 */
vector<string> prompt_file_dialog_locked_to_folder(
    const string &folder, const string &title,
    const string &patterns, const int mode, FILE_DIALOG_RESULTS* result
) {
    vector<string> f =
        prompt_file_dialog(folder + "/", title, patterns, mode);
        
    if(f.empty() || f[0].empty()) {
        *result = FILE_DIALOG_RES_CANCELED;
        return vector<string>();
    }
    
    for(size_t fi = 0; fi < f.size(); ++fi) {
        size_t folder_pos = f[0].find(folder);
        if(folder_pos == string::npos) {
            //This isn't in the specified folder!
            *result = FILE_DIALOG_RES_WRONG_FOLDER;
            return vector<string>();
        } else {
            f[fi] =
                f[fi].substr(folder_pos + folder.size() + 1, string::npos);
        }
    }
    
    *result = FILE_DIALOG_RES_SUCCESS;
    return f;
}


/* ----------------------------------------------------------------------------
 * Basically, it destroys and recreates a bitmap.
 * The main purpose of this is to update its mipmap.
 * b:
 *   The bitmap.
 */
ALLEGRO_BITMAP* recreate_bitmap(ALLEGRO_BITMAP* b) {
    ALLEGRO_BITMAP* fixed_mipmap = al_clone_bitmap(b);
    al_destroy_bitmap(b);
    return fixed_mipmap;
}


/* ----------------------------------------------------------------------------
 * Reports a fatal error to the user and shuts down the program.
 * s:
 *   String explaining the error.
 * dn:
 *   File to log the error into, if any.
 */
void report_fatal_error(const string &s, data_node* dn) {
    log_error(s, dn);
    
    show_message_box(
        NULL, "Fatal error!",
        "Pikifen has encountered a fatal error!",
        s.c_str(),
        NULL,
        ALLEGRO_MESSAGEBOX_ERROR
    );
    
    exit(-1);
    
}


/* ----------------------------------------------------------------------------
 * Converts a string to an Allegro color.
 * Components are separated by spaces, and the final one (alpha) is optional.
 * s:
 *   String to convert.
 */
ALLEGRO_COLOR s2c(const string &s) {
    string s2 = s;
    s2 = trim_spaces(s2);
    
    unsigned char alpha = 255;
    vector<string> components = split(s2);
    if(components.size() >= 2) alpha = s2i(components[1]);
    
    if(s2 == "nothing") return al_map_rgba(0,   0,   0,   0);
    if(s2 == "none")    return al_map_rgba(0,   0,   0,   0);
    if(s2 == "black")   return al_map_rgba(0,   0,   0,   alpha);
    if(s2 == "gray")    return al_map_rgba(128, 128, 128, alpha);
    if(s2 == "grey")    return al_map_rgba(128, 128, 128, alpha);
    if(s2 == "white")   return map_alpha(alpha);
    if(s2 == "yellow")  return al_map_rgba(255, 255, 0,   alpha);
    if(s2 == "orange")  return al_map_rgba(255, 128, 0,   alpha);
    if(s2 == "brown")   return al_map_rgba(128, 64,  0,   alpha);
    if(s2 == "red")     return al_map_rgba(255, 0,   0,   alpha);
    if(s2 == "violet")  return al_map_rgba(255, 0,   255, alpha);
    if(s2 == "purple")  return al_map_rgba(128, 0,   255, alpha);
    if(s2 == "blue")    return al_map_rgba(0,   0,   255, alpha);
    if(s2 == "cyan")    return al_map_rgba(0,   255, 255, alpha);
    if(s2 == "green")   return al_map_rgba(0,   255, 0,   alpha);
    
    ALLEGRO_COLOR c =
        al_map_rgba(
            ((components.size() > 0) ? s2i(components[0]) : 0),
            ((components.size() > 1) ? s2i(components[1]) : 0),
            ((components.size() > 2) ? s2i(components[2]) : 0),
            ((components.size() > 3) ? s2i(components[3]) : 255)
        );
    return c;
}


/* ----------------------------------------------------------------------------
 * Converts a string to a point.
 * s:
 *   String to convert.
 * z:
 *   If not NULL, the third word is placed here.
 */
point s2p(const string &s, float* z) {
    vector<string> words = split(s);
    point p;
    if(words.size() >= 1) {
        p.x = s2f(words[0]);
    }
    if(words.size() >= 2) {
        p.y = s2f(words[1]);
    }
    if(z && words.size() >= 3) {
        *z = s2f(words[2]);
    }
    return p;
}


/* ----------------------------------------------------------------------------
 * Sanitizes a file name (or part of it), such that it doesn't use any
 * weird characters.
 * Do not use on paths, since colons, slashes, and backslashes will be replaced!
 * s:
 *   File name to sanitize.
 */
string sanitize_file_name(const string &s) {
    string ret;
    ret.reserve(s.size());
    for(size_t c = 0; c < s.size(); ++c) {
        if(
            (s[c] >= 'A' && s[c] <= 'Z') ||
            (s[c] >= 'a' && s[c] <= 'z') ||
            (s[c] >= '0' && s[c] <= '9') ||
            s[c] == '-' ||
            s[c] == ' '
        ) {
            ret.push_back(s[c]);
        } else {
            ret.push_back('_');
        }
    }
    return ret;
}


/* ----------------------------------------------------------------------------
 * Saves the maker tools settings.
 */
void save_maker_tools() {
    data_node file("", "");
    
    file.add(
        new data_node("enabled", b2s(game.maker_tools.enabled))
    );
    
    for(unsigned char k = 0; k < 20; k++) {
        string tool_key;
        if(k < 10) {
            //The first ten indexes are the F2 - F11 keys.
            tool_key = "f" + i2s(k + 2);
        } else {
            //The second ten indexes are the 0 - 9 keys.
            tool_key = i2s(k - 10);
        }
        string tool_name = MAKER_TOOL_NAMES[game.maker_tools.keys[k]];
        
        file.add(new data_node(tool_key, tool_name));
    }
    
    file.add(
        new data_node(
            "area_image_mobs", b2s(game.maker_tools.area_image_mobs)
        )
    );
    file.add(
        new data_node(
            "area_image_shadows", b2s(game.maker_tools.area_image_shadows)
        )
    );
    file.add(
        new data_node(
            "area_image_size", i2s(game.maker_tools.area_image_size)
        )
    );
    file.add(
        new data_node(
            "change_speed_multiplier", f2s(game.maker_tools.change_speed_mult)
        )
    );
    file.add(
        new data_node(
            "mob_hurting_percentage",
            f2s(game.maker_tools.mob_hurting_ratio * 100)
        )
    );
    
    file.add(
        new data_node(
            "auto_start_option", game.maker_tools.auto_start_option
        )
    );
    file.add(
        new data_node(
            "auto_start_mode", game.maker_tools.auto_start_mode
        )
    );
    file.add(
        new data_node(
            "performance_monitor", b2s(game.maker_tools.use_perf_mon)
        )
    );
    
    file.save_file(MAKER_TOOLS_FILE_PATH, true, true);
}


/* ----------------------------------------------------------------------------
 * Saves the player's options.
 */
void save_options() {
    data_node file("", "");
    
    //Save the standard options.
    game.options.save(&file);
    
    //Also add the animation editor history.
    for(
        size_t h = 0; h < game.states.animation_ed->history.size(); ++h
    ) {
        file.add(
            new data_node(
                "animation_editor_history_" + i2s(h + 1),
                game.states.animation_ed->history[h]
            )
        );
    }
    
    //Finally, save.
    file.save_file(OPTIONS_FILE_PATH, true, true);
}


/* ----------------------------------------------------------------------------
 * Saves the current backbuffer onto a file.
 * In other words, dumps a screenshot.
 */
void save_screenshot() {
    string base_file_name = "Screenshot_" + get_current_time(true);
    
    //Check if a file with this name already exists.
    vector<string> files = folder_to_vector(USER_DATA_FOLDER_PATH, false);
    size_t variant_nr = 1;
    string final_file_name = base_file_name;
    bool valid_name = false;
    
    do {
    
        if(
            find(files.begin(), files.end(), final_file_name + ".png")
            == files.end()
        ) {
            //File name not found.
            //Go ahead and create a screenshot with this name.
            valid_name = true;
        } else {
            variant_nr++;
            final_file_name = base_file_name + " " + i2s(variant_nr);
        }
        
    } while(!valid_name);
    
    al_save_bitmap(
        (USER_DATA_FOLDER_PATH + "/" + final_file_name + ".png").c_str(),
        al_get_backbuffer(game.display)
    );
}


/* ----------------------------------------------------------------------------
 * Given a list of items, chooses which item comes next geometrically in the
 * specified direction. Useful for menus with several buttons the player can
 * select multidirectionally in.
 * Also, it loops around.
 * item_coordinates:
 *   Vector with the center coordinates of all items.
 * selected_item:
 *   Index of the selected item.
 * direction:
 *   Angle specifying the direction.
 * loop_region:
 *   Width and height of the loop region.
 */
size_t select_next_item_directionally(
    const vector<point> item_coordinates, const size_t selected_item,
    const float direction, const point &loop_region
) {
    const float MIN_BLINDSPOT_ANGLE = TAU * 0.17;
    const float MAX_BLINDSPOT_ANGLE = TAU * 0.33;
    
    float normalized_dir = normalize_angle(direction);
    const point &sel_coords = item_coordinates[selected_item];
    float best_score = FLT_MAX;
    size_t best_item = selected_item;
    
    //Check each item that isn't the current one.
    for(size_t i = 0; i < item_coordinates.size(); ++i) {
    
        if(i == selected_item) continue;
        
        point i_base_coords = item_coordinates[i];
        
        //Get the standard coordinates for this item, and make them relative.
        point i_coords = i_base_coords;
        i_coords = i_coords - sel_coords;
        
        //Rotate the coordinates such that the specified direction
        //lands to the right.
        i_coords = rotate_point(i_coords, -normalized_dir);
        
        //Check if it's between the blind spot angles.
        //We get the same result whether the Y is positive or negative,
        //so let's simplify things and make it positive.
        float rel_angle =
            get_angle(point(0, 0), point(i_coords.x, fabs(i_coords.y)));
        if(
            rel_angle >= MIN_BLINDSPOT_ANGLE &&
            rel_angle <= MAX_BLINDSPOT_ANGLE
        ) {
            //If so, never let this item be chosen, no matter what. This is
            //useful to stop a list of items with no vertical variance from
            //picking another item when the direction is up, for instance.
            continue;
        }
        
        if(i_coords.x > 0.0f) {
            //If this item is in front of the selected one,
            //give it a score like normal.
            float score = i_coords.x + fabs(i_coords.y);
            if(score < best_score) {
                best_score = score;
                best_item = i;
            }
            
        } else {
            //If the item is behind, we'll need to loop its coordinates
            //and score those loop coordinates that land in front.
            //Unfortunately, there's no way to know how the coordinates
            //should be looped in order to land in front of the selected
            //item, so we should just check all loop variations: above, below
            //to the left, to the right, and combinations.
            
            for(char c = -1; c < 2; ++c) {
                for(char r = -1; r < 2; ++r) {
                
                    //If it's the same "screen" as the regular one,
                    //forget it, since we already checked above.
                    if(c == 0 && r == 0) {
                        continue;
                    }
                    
                    //Get the coordinates in this parallel region, and make
                    //them relative.
                    i_coords = i_base_coords;
                    i_coords.x += loop_region.x * c;
                    i_coords.y += loop_region.y * r;
                    i_coords = i_coords - sel_coords;
                    
                    //Rotate the coordinates such that the specified direction
                    //lands to the right.
                    i_coords = rotate_point(i_coords, -normalized_dir);
                    
                    //If these coordinates are behind the selected item,
                    //they cannot be selected.
                    if(i_coords.x < 0.0f) {
                        continue;
                    }
                    
                    //Finally, figure out if this is the new best item.
                    float score = i_coords.x + fabs(i_coords.y);
                    if(score < best_score) {
                        best_score = score;
                        best_item = i;
                    }
                }
            }
        }
    }
    
    return best_item;
}


/* ----------------------------------------------------------------------------
 * Returns a vector with all items inside a semicolon-separated list.
 * s:
 *   The string containing the list.
 * sep:
 *   Separator to use, in case you need something else. Default is semicolon.
 */
vector<string> semicolon_list_to_vector(const string &s, const string &sep) {
    vector<string> parts = split(s, sep);
    for(size_t p = 0; p < parts.size(); ++p) {
        parts[p] = trim_spaces(parts[p]);
    }
    return parts;
}


/* ----------------------------------------------------------------------------
 * Shows a native message box. It is better to call this rather than
 * al_show_native_message_box() directly because it does not reset the locale
 * after it is done.
 * display:
 *   Display responsible for this dialog.
 * title:
 *   Title to display on the dialog.
 * heading:
 *   Heading text to display on the dialog.
 * text:
 *   Main text to display on the dialog.
 * buttons:
 *   Buttons the user can press.
 * flags:
 *   al_show_native_message_box flags.
 */
int show_message_box(
    ALLEGRO_DISPLAY* display, char const* title, char const* heading,
    char const* text, char const* buttons, int flags
) {
    int ret =
        al_show_native_message_box(
            display, title, heading, text, buttons, flags
        );
    //Reset the locale, which gets set by Allegro's native dialogs...
    //and breaks s2f().
    setlocale(LC_ALL, "C");
    
    return ret;
}


/* ----------------------------------------------------------------------------
 * Handles a system signal.
 * signum:
 *   Signal number.
 */
void signal_handler(const int signum) {
    volatile static bool already_handling_signal = false;
    
    if(already_handling_signal) {
        //This stops an infinite loop if there's a signal raise
        //inside this function. It shouldn't happen, but better be safe.
        exit(signum);
    }
    already_handling_signal = true;
    
    string bt_str = "Backtrace:\n";
    vector<string> bt = get_backtrace();
    for(size_t s = 0; s < bt.size(); ++s) {
        bt_str += "    " + bt[s] + "\n";
    }
    if(bt_str.back() == '\n') {
        bt_str.pop_back();
    }
    string signal_name(strsignal(signum));
    string type_str = "Signal " + i2s(signum) + " (" + signal_name + ")";
    
    crash(type_str, bt_str, signum);
}


/* ----------------------------------------------------------------------------
 * Spews out a Pikmin from a given point. Used by Onions and converters.
 * pos:
 *   Point of origin.
 * z:
 *   Z of the point of origin.
 * pik_type:
 *   Type of the Pikmin to spew out.
 * angle:
 *   Direction in which to spew.
 * horizontal_speed:
 *   Horizontal speed in which to spew.
 * vertical_speed:
 *   Vertical speed in which to spew.
 */
void spew_pikmin_seed(
    const point pos, const float z, pikmin_type* pik_type,
    const float angle, const float horizontal_speed, const float vertical_speed
) {
    pikmin* new_pikmin =
        (
            (pikmin*)
            create_mob(
                game.mob_categories.get(MOB_CATEGORY_PIKMIN),
                pos, pik_type, angle, "", nullptr, PIKMIN_STATE_SEED
            )
        );
    new_pikmin->z = z;
    new_pikmin->speed.x = cos(angle) * horizontal_speed;
    new_pikmin->speed.y = sin(angle) * horizontal_speed;
    new_pikmin->speed_z = vertical_speed;
    new_pikmin->maturity = 0;
}


/* ----------------------------------------------------------------------------
 * Standardizes a path, making it use forward slashes instead of backslashes,
 * and removing excess slashes at the end.
 * path:
 *   Path to standardize.
 */
string standardize_path(const string &path) {
    string res = replace_all(path, "\\", "/");
    if(res.back() == '/') res.pop_back();
    return res;
}


/* ----------------------------------------------------------------------------
 * Starts the display of a text message.
 * If the text is empty, it closes the message box.
 * Any newline characters or slashes followed by n ("\n") will be used to
 * separate the message into lines.
 * text:
 *   Text to display.
 * speaker_bmp:
 *   Bitmap representing the speaker.
 */
void start_message(string text, ALLEGRO_BITMAP* speaker_bmp) {
    if(!text.empty()) {
        string final_text = unescape_string(text);
        game.states.gameplay->msg_box =
            new msg_box_info(final_text, speaker_bmp);
        game.states.gameplay->hud->gui.start_animation(
            GUI_MANAGER_ANIM_IN_TO_OUT,
            gameplay_state::MENU_ENTRY_HUD_MOVE_TIME
        );
    } else {
        delete game.states.gameplay->msg_box;
        game.states.gameplay->msg_box = NULL;
        game.states.gameplay->hud->gui.start_animation(
            GUI_MANAGER_ANIM_OUT_TO_IN,
            gameplay_state::MENU_EXIT_HUD_MOVE_TIME
        );
    }
}


#if defined(_WIN32)

/* ----------------------------------------------------------------------------
 * An implementation of strsignal from POSIX.
 * signum:
 *   Signal number.
 */
string strsignal(const int signum) {
    switch(signum) {
    case SIGINT: {
        return "SIGINT";
    } case SIGILL: {
        return "SIGILL";
    } case SIGFPE: {
        return "SIGFPE";
    } case SIGSEGV: {
        return "SIGSEGV";
    } case SIGTERM: {
        return "SIGTERM";
    } case SIGBREAK: {
        return "SIGBREAK";
    } case SIGABRT: {
        return "SIGABRT";
    } case SIGABRT_COMPAT: {
        return "SIGABRT_COMPAT";
    } default: {
        return "Unknown";
    }
    }
}


#endif //if defined(_WIN32)


/* ----------------------------------------------------------------------------
 * Unescapes a user string. This converts two backslashes into one, and
 * converts backslash n into a newline character.
 * s:
 *   String to unescape.
 */
string unescape_string(const string &s) {
    if(s.empty()) return s;
    
    string ret;
    ret.reserve(s.size());
    for(size_t c = 0; c < s.size() - 1;) {
        if(s[c] == '\\') {
            switch(s[c + 1]) {
            case 'n': {
                ret.push_back('\n');
                c += 2;
                break;
            } case '\\': {
                ret.push_back('\\');
                c += 2;
                break;
            } default: {
                ret.push_back('\\');
                ++c;
                break;
            }
            }
        } else {
            ret.push_back(s[c]);
            ++c;
        }
    }
    ret.push_back(s.back());
    return ret;
}


/* ----------------------------------------------------------------------------
 * Returns a string that's a join of the strings in the specified vector,
 * but only past a certain position. The strings are joined with a space
 * character.
 * v:
 *   The vector of strings.
 * pos:
 *   Use the string at this position and onward.
 */
string vector_tail_to_string(const vector<string> &v, const size_t pos) {
    string result = v[pos];
    for(size_t p = pos + 1; p < v.size(); ++p) {
        result += " " + v[p];
    }
    return result;
}


/* ----------------------------------------------------------------------------
 * Deletes all "non-important" files inside of a folder.
 * Then, if the folder ends up empty, also deletes the folder.
 * folder_path:
 *   Path to the folder to wipe.
 * non_important_files:
 *   List of files that can be deleted.
 */
WIPE_FOLDER_RESULTS wipe_folder(
    const string &folder_path, const vector<string> &non_important_files
) {
    ALLEGRO_FS_ENTRY* folder =
        al_create_fs_entry(folder_path.c_str());
    if(!folder || !al_open_directory(folder)) {
        return WIPE_FOLDER_RESULT_NOT_FOUND;
    }
    
    bool has_important_files = false;
    bool has_folders = false;
    bool non_important_file_delete_error = false;
    bool folder_delete_error = false;
    
    ALLEGRO_FS_ENTRY* entry = al_read_directory(folder);
    while(entry) {
        if((al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR) > 0) {
            has_folders = true;
            
        } else {
            string entry_name =
                standardize_path(al_get_fs_entry_name(entry));
                
            //Only save what's after the final slash.
            size_t pos = entry_name.find_last_of("/");
            if(pos != string::npos) {
                entry_name =
                    entry_name.substr(pos + 1, entry_name.size() - pos - 1);
            }
            
            if(
                std::find(
                    non_important_files.begin(),
                    non_important_files.end(),
                    entry_name
                ) == non_important_files.end()
            ) {
                //Name not found in the non-important file list.
                has_important_files = true;
            } else {
                if(!al_remove_fs_entry(entry)) {
                    non_important_file_delete_error = true;
                }
            }
            
        }
        
        al_destroy_fs_entry(entry);
        entry = al_read_directory(folder);
    }
    
    al_close_directory(folder);
    
    if(
        !has_important_files &&
        !has_folders &&
        !non_important_file_delete_error
    ) {
        if(!al_remove_fs_entry(folder)) {
            folder_delete_error = true;
        }
    }
    
    al_destroy_fs_entry(folder);
    
    if(non_important_file_delete_error || folder_delete_error) {
        return WIPE_FOLDER_RESULT_DELETE_ERROR;
    }
    if(has_important_files || has_folders) {
        return WIPE_FOLDER_RESULT_HAS_IMPORTANT;
    }
    return WIPE_FOLDER_RESULT_OK;
}
