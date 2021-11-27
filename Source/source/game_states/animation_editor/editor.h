/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general animation editor-related functions.
 */

#ifndef ANIMATION_EDITOR_INCLUDED
#define ANIMATION_EDITOR_INCLUDED

#include <string>

#include "../editor.h"
#include "../../imgui/imgui_impl_allegro5.h"


class animation_editor : public editor {
public:

    static const size_t HISTORY_SIZE;
    
    //Automatically load this animation file upon boot-up of the editor, if any.
    string auto_load_anim;
    //History for the last files that were opened.
    vector<string> history;
    
    void update_history(const string &n);
    
    void do_logic();
    void do_drawing();
    void draw_canvas();
    void load();
    void unload();
    virtual string get_name() const;
    
    animation_editor();
    
private:

private:

    enum EDITOR_STATES {
        EDITOR_STATE_MAIN,
        EDITOR_STATE_ANIMATION,
        EDITOR_STATE_SPRITE,
        EDITOR_STATE_BODY_PART,
        EDITOR_STATE_HITBOXES,
        EDITOR_STATE_SPRITE_BITMAP,
        EDITOR_STATE_SPRITE_TRANSFORM,
        EDITOR_STATE_TOP,
        EDITOR_STATE_TOOLS,
    };
    
    static const float KEYBOARD_PAN_AMOUNT;
    static const float HITBOX_MIN_RADIUS;
    static const size_t TIMELINE_HEADER_HEIGHT;
    static const size_t TIMELINE_HEIGHT;
    static const size_t TIMELINE_LOOP_TRI_SIZE;
    static const size_t TIMELINE_PADDING;
    static const float TOP_MIN_SIZE;
    static const float ZOOM_MAX_LEVEL_EDITOR;
    static const float ZOOM_MIN_LEVEL_EDITOR;
    
    //Currently loaded animation database.
    animation_database anims;
    //Is the current animation playing?
    bool anim_playing;
    //Does the animation exist on disk, or RAM only?
    bool animation_exists_on_disk;
    //Can the user use the "save" button?
    bool can_save;
    //Is the sprite comparison mode on?
    bool comparison;
    //Is the comparison sprite above the working sprite?
    bool comparison_above;
    //Is the comparison sprite meant to blink?
    bool comparison_blink;
    //Is the blinking comparison sprite currently visible?
    bool comparison_blink_show;
    //Time left until the blinking comparison sprite's visibility is swapped.
    timer comparison_blink_timer;
    //Comparison sprite to use in sprite comparison mode.
    sprite* comparison_sprite;
    //Is the comparison sprite mode tinting the sprites?
    bool comparison_tint;
    //Current animation.
    animation* cur_anim;
    //Index number of the current frame of animation.
    size_t cur_frame_nr;
    //Time spent in the current frame of animation.
    float cur_frame_time;
    //Current hitbox.
    hitbox* cur_hitbox;
    //The alpha is calculated using the sine of this value.
    float cur_hitbox_alpha;
    //Index number of the current hitbox.
    size_t cur_hitbox_nr;
    //Current maturity to display on the Pikmin's top.
    unsigned char cur_maturity;
    //Current sprite.
    sprite* cur_sprite;
    //Keep the aspect ratio when resizing the current sprite?
    bool cur_sprite_keep_aspect_ratio;
    //The current transformation widget.
    transformation_widget cur_transformation_widget;
    //File path of the file currently being edited.
    string file_path;
    //Cache with the names of all global animation files (sans extension).
    vector<string> global_anim_files_cache;
    //Is the grid visible?
    bool grid_visible;
    //Are the hitboxes currently visible?
    bool hitboxes_visible;
    //Last file used as for a spritesheet.
    string last_spritesheet_used;
    //Mob type of the currently loaded animation file, if any.
    mob_type* loaded_mob_type;
    //Is the mob radius visible?
    bool mob_radius_visible;
    //Is the Pikmin silhouette visible?
    bool pikmin_silhouette_visible;
    //Before entering the sprite bitmap state, this was the camera position.
    point pre_sprite_bmp_cam_pos;
    //Before entering the sprite bitmap state, this was the camera zoom.
    float pre_sprite_bmp_cam_zoom;
    //Should the load dialog's GUI variables be reset?
    bool reset_load_dialog;
    //Is side view on?
    bool side_view;
    //Is the add mode on in the sprite bitmap state?
    bool sprite_bmp_add_mode;
    //Top bitmaps for the current Pikmin type.
    ALLEGRO_BITMAP* top_bmp[N_MATURITIES];
    //Keep the aspect ratio when resizing the Pikmin top?
    bool top_keep_aspect_ratio;
    
    //Position of some important widgets.
    point load_widget_pos;
    point reload_widget_pos;
    point quit_widget_pos;
    
    
    //General functions.
    void center_camera_on_sprite_bitmap(const bool instant);
    void change_state(const EDITOR_STATES new_state);
    void close_load_dialog();
    void close_options_dialog();
    static void draw_canvas_imgui_callback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    void draw_comparison();
    void draw_side_view_hitbox(
        hitbox* h_ptr, const ALLEGRO_COLOR &color,
        const ALLEGRO_COLOR &outline_color, const float outline_thickness
    );
    void draw_side_view_pikmin_silhouette(const float x_offset);
    void draw_side_view_sprite(sprite* s);
    void draw_timeline();
    void draw_top_down_view_hitbox(
        hitbox* h_ptr, const ALLEGRO_COLOR &color,
        const ALLEGRO_COLOR &outline_color, const float outline_thickness
    );
    void draw_top_down_view_mob_radius(mob_type* mt);
    void draw_top_down_view_pikmin_silhouette(const float x_offset);
    void draw_top_down_view_sprite(sprite* s);
    void enter_side_view();
    void exit_side_view();
    float get_cursor_timeline_time();
    string get_path_short_name(const string &p) const;
    void handle_lmb_drag_in_timeline();
    void import_animation_data(const string &name);
    void import_sprite_file_data(const string &name);
    void import_sprite_hitbox_data(const string &name);
    void import_sprite_top_data(const string &name);
    void import_sprite_transformation_data(const string &name);
    bool is_cursor_in_timeline();
    void load_animation_database(const bool should_update_history);
    void open_load_dialog();
    void open_options_dialog();
    void pick_animation(
        const string &name, const string &category, const bool is_new
    );
    void pick_sprite(
        const string &name, const string &category, const bool is_new
    );
    void press_grid_button();
    void press_hitboxes_button();
    void press_load_button();
    void press_mob_radius_button();
    void press_pikmin_silhouette_button();
    void press_play_animation_button();
    void press_quit_button();
    void press_reload_button();
    void press_save_button();
    void process_gui();
    void process_gui_control_panel();
    void process_gui_hitbox_hazards();
    void process_gui_load_dialog();
    void process_gui_options_dialog();
    void process_gui_panel_animation();
    void process_gui_panel_body_part();
    void process_gui_panel_main();
    void process_gui_panel_options();
    void process_gui_panel_sprite();
    void process_gui_panel_sprite_bitmap();
    void process_gui_panel_sprite_hitboxes();
    void process_gui_panel_sprite_top();
    void process_gui_panel_sprite_transform();
    void process_gui_panel_tools();
    void process_gui_menu_bar();
    void process_gui_status_bar();
    void process_gui_toolbar();
    void rename_animation(animation* a, const string &new_name);
    void rename_body_part(body_part* p, const string &new_name);
    void rename_sprite(sprite* s, const string &new_name);
    void resize_everything(const float mult);
    void resize_sprite(sprite* s, const float mult);
    void save_animation_database();
    void set_all_sprite_scales(const float scale);
    void set_best_frame_sprite();
    void sprite_bmp_flood_fill(
        ALLEGRO_BITMAP* bmp, bool* selection_pixels, const int x, const int y
    );
    void update_cur_hitbox();
    void update_hitboxes();
    void update_stats();
    
    //Input handler functions.
    void handle_key_char_canvas(const ALLEGRO_EVENT &ev);
    void handle_key_down_anywhere(const ALLEGRO_EVENT &ev);
    void handle_key_down_canvas(const ALLEGRO_EVENT &ev);
    void handle_lmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_lmb_down(const ALLEGRO_EVENT &ev);
    void handle_lmb_drag(const ALLEGRO_EVENT &ev);
    void handle_lmb_up(const ALLEGRO_EVENT &ev);
    void handle_mmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_mmb_down(const ALLEGRO_EVENT &ev);
    void handle_mmb_drag(const ALLEGRO_EVENT &ev);
    void handle_mouse_update(const ALLEGRO_EVENT &ev);
    void handle_mouse_wheel(const ALLEGRO_EVENT &ev);
    void handle_rmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_rmb_down(const ALLEGRO_EVENT &ev);
    void handle_rmb_drag(const ALLEGRO_EVENT &ev);
    void pan_cam(const ALLEGRO_EVENT &ev);
    void reset_cam_xy();
    void reset_cam_zoom();
};


#endif //ifndef ANIMATION_EDITOR_INCLUDED
