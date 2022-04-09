/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * In-world HUD class and in-world HUD-related functions.
 */

#include "in_world_hud.h"

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../mobs/mob.h"
#include "../../utils/allegro_utils.h"
#include "../../utils/string_utils.h"


//How long it takes to animate the numbers growing.
const float in_world_fraction::GROW_JUICE_DURATION = 0.3f;
//How much to grow when performing a juicy grow animation.
const float in_world_fraction::GROW_JUICE_AMOUNT = 0.06f;
//How long it takes to animate the numbers flashing.
const float in_world_fraction::REQ_MET_JUICE_DURATION = 0.5f;
//How much to grow when performing a requirement met juicy grow animation.
const float in_world_fraction::REQ_MET_GROW_JUICE_AMOUNT = 0.12f;
//How long it takes to fade in.
const float in_world_fraction::TRANSITION_IN_DURATION = 0.4f;
//How long it takes to fade out.
const float in_world_fraction::TRANSITION_OUT_DURATION = 0.5f;
//Multiply health wheel speed by this.
const float in_world_health_wheel::SMOOTHNESS_MULT = 6.0f;
//How long it takes to fade in.
const float in_world_health_wheel::TRANSITION_IN_DURATION = 0.2f;
//How long it takes to fade out.
const float in_world_health_wheel::TRANSITION_OUT_DURATION = 1.5f;


/* ----------------------------------------------------------------------------
 * Creates an in-world fraction.
 */
in_world_fraction::in_world_fraction(mob* m) :
    in_world_hud_item(m),
    value_number(0),
    requirement_number(0),
    color(COLOR_BLACK),
    grow_juice_timer(0.0f),
    req_met_juice_timer(0.0f) {
    
    transition_timer = TRANSITION_IN_DURATION;
}


/* ----------------------------------------------------------------------------
 * Draws an in-world fraction.
 */
void in_world_fraction::draw() {
    //Padding between mob and fraction.
    const float PADDING = 8.0f;
    
    float alpha_mult = 1.0f;
    float size_mult = 1.0f;
    
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        float timer_ratio = 1 - (transition_timer / TRANSITION_IN_DURATION);
        alpha_mult = timer_ratio;
        size_mult = ease(EASE_OUT, timer_ratio) * 0.5 + 0.5;
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        alpha_mult = transition_timer / TRANSITION_OUT_DURATION;
        break;
    }
    default: {
        break;
    }
    }
    
    if(grow_juice_timer > 0.0f) {
        float anim_ratio = 1 - (grow_juice_timer / GROW_JUICE_DURATION);
        anim_ratio = ease(EASE_UP_AND_DOWN, anim_ratio);
        size_mult += GROW_JUICE_AMOUNT * anim_ratio;
    }
    
    ALLEGRO_COLOR final_color;
    if(req_met_juice_timer > 0.0f) {
        final_color =
            interpolate_color(
                req_met_juice_timer, 0.0f, REQ_MET_JUICE_DURATION,
                color, COLOR_WHITE
            );
            
        float anim_ratio = 1 - (req_met_juice_timer / REQ_MET_JUICE_DURATION);
        anim_ratio = ease(EASE_UP_AND_DOWN, anim_ratio);
        size_mult += REQ_MET_GROW_JUICE_AMOUNT * anim_ratio;
    } else {
        final_color = color;
    }
    final_color.a *= alpha_mult;
    
    if(requirement_number > 0) {
        point pos(m->pos.x, m->pos.y - m->radius - PADDING);
        draw_fraction(
            pos,
            value_number, requirement_number,
            final_color, size_mult
        );
    } else {
        point pos(
            m->pos.x,
            m->pos.y - m->radius -
            al_get_font_line_height(game.fonts.standard) - PADDING
        );
        draw_scaled_text(
            game.fonts.standard,
            final_color, pos,
            point(size_mult, size_mult),
            ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER, i2s(value_number)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Sets the color.
 */
void in_world_fraction::set_color(const ALLEGRO_COLOR &new_color) {
    if(color == new_color) return;
    
    color = new_color;
    grow_juice_timer = GROW_JUICE_DURATION;
}


/* ----------------------------------------------------------------------------
 * Sets the requirement number.
 */
void in_world_fraction::set_requirement_number(const float new_req_nr) {
    if(requirement_number == new_req_nr) return;
    
    bool req_was_met = value_number >= requirement_number;
    requirement_number = new_req_nr;
    
    if(
        requirement_number > 0.0f &&
        !req_was_met &&
        value_number >= requirement_number
    ) {
        req_met_juice_timer = REQ_MET_JUICE_DURATION;
    } else {
        grow_juice_timer = GROW_JUICE_DURATION;
    }
}


/* ----------------------------------------------------------------------------
 * Sets the value number.
 */
void in_world_fraction::set_value_number(const float new_value_nr) {
    if(value_number == new_value_nr) return;
    
    bool req_was_met = value_number >= requirement_number;
    
    value_number = new_value_nr;
    
    if(
        requirement_number > 0.0f &&
        !req_was_met &&
        value_number >= requirement_number
    ) {
        req_met_juice_timer = REQ_MET_JUICE_DURATION;
    } else {
        grow_juice_timer = GROW_JUICE_DURATION;
    }
}


/* ----------------------------------------------------------------------------
 * Starts fading away.
 */
void in_world_fraction::start_fading() {
    if(transition == IN_WORLD_HUD_TRANSITION_OUT) {
        return;
    }
    transition = IN_WORLD_HUD_TRANSITION_OUT;
    transition_timer = TRANSITION_OUT_DURATION;
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void in_world_fraction::tick(const float delta_t) {
    in_world_hud_item::tick(delta_t);
    if(grow_juice_timer > 0.0f) {
        grow_juice_timer -= delta_t;
    }
    if(req_met_juice_timer > 0.0f) {
        req_met_juice_timer -= delta_t;
    }
}


/* ----------------------------------------------------------------------------
 * Creates an in-world health wheel.
 */
in_world_health_wheel::in_world_health_wheel(mob* m) :
    in_world_hud_item(m),
    visible_ratio(0.0f) {
    
    if(m->type->max_health > 0.0f) {
        visible_ratio = m->health / m->type->max_health;
    }
    transition_timer = TRANSITION_IN_DURATION;
}


/* ----------------------------------------------------------------------------
 * Draws an in-world health wheel.
 */
void in_world_health_wheel::draw() {
    //Standard opacity.
    const float OPACITY = 0.85f;
    //Padding between mob and wheel.
    const float PADDING = 4.0f;
    
    float alpha_mult = 1.0f;
    float size_mult = 1.0f;
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        float timer_ratio = 1 - (transition_timer / TRANSITION_IN_DURATION);
        alpha_mult = timer_ratio;
        size_mult = ease(EASE_OUT, timer_ratio) * 0.5 + 0.5;
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        alpha_mult = transition_timer / TRANSITION_OUT_DURATION;
        break;
    }
    default: {
        break;
    }
    }
    
    float radius = DEF_HEALTH_WHEEL_RADIUS * size_mult;
    draw_health(
        point(
            m->pos.x,
            m->pos.y - m->radius -
            radius - PADDING
        ),
        visible_ratio,
        OPACITY * alpha_mult,
        radius
    );
}


/* ----------------------------------------------------------------------------
 * Starts fading away.
 */
void in_world_health_wheel::start_fading() {
    if(transition == IN_WORLD_HUD_TRANSITION_OUT) {
        return;
    }
    transition = IN_WORLD_HUD_TRANSITION_OUT;
    transition_timer = TRANSITION_OUT_DURATION;
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void in_world_health_wheel::tick(const float delta_t) {
    in_world_hud_item::tick(delta_t);
    
    if(m->type->max_health == 0.0f) return;
    
    visible_ratio +=
        ((m->health / m->type->max_health) - visible_ratio) *
        (SMOOTHNESS_MULT * delta_t);
}


/* ----------------------------------------------------------------------------
 * Creates an in-world HUD item.
 * m:
 *   Mob it belongs to.
 */
in_world_hud_item::in_world_hud_item(mob* m) :
    m(m),
    transition(IN_WORLD_HUD_TRANSITION_IN),
    transition_timer(0.0f),
    to_delete(false) {
    
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void in_world_hud_item::tick(const float delta_t) {
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        transition_timer -= delta_t;
        if(transition_timer <= 0.0f) {
            transition = IN_WORLD_HUD_TRANSITION_NONE;
        }
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        transition_timer -= delta_t;
        if(transition_timer <= 0.0f) {
            to_delete = true;
        }
        break;
    }
    default: {
        break;
    }
    }
}
