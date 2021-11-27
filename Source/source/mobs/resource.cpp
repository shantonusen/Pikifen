/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Resource class and resource-related functions.
 */

#include "resource.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"


/* ----------------------------------------------------------------------------
 * Creates a resource.
 * pos:
 *   Starting coordinates.
 * type:
 *   Resource type this mob belongs to.
 * angle:
 *   Starting angle.
 */
resource::resource(const point &pos, resource_type* type, const float angle) :
    mob(pos, type, angle),
    res_type(type),
    origin_pile(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Draws a resource.
 */
void resource::draw_mob() {
    sprite* s_ptr = get_cur_sprite();
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    ALLEGRO_COLOR delivery_color = map_gray(0);
    float delivery_time_ratio_left = LARGE_FLOAT;
    
    if(fsm.cur_state->id == ENEMY_EXTRA_STATE_BEING_DELIVERED) {
        delivery_color = game.config.carrying_color_move;
        delivery_time_ratio_left = script_timer.get_ratio_left();
    }
    
    get_sprite_bitmap_effects(
        s_ptr, &eff, true, true,
        delivery_time_ratio_left, delivery_color
    );
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
}
