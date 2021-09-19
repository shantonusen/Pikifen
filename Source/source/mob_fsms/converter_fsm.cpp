/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Converter finite state machine logic.
 */

#include "converter_fsm.h"

#include "../functions.h"
#include "../game.h"
#include "../mobs/converter.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the converter's logic.
 * typ:
 *   Mob type to create the finite state machine for.
 */
void converter_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", CONVERTER_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::become_idle);
        }
        efc.new_event(MOB_EV_THROWN_PIKMIN_LANDED); {
            efc.run(converter_fsm::handle_pikmin);
        }
        efc.new_event(MOB_EV_TOUCHED_OBJECT); {
            efc.run(converter_fsm::handle_object_touch);
        }
    }
    
    efc.new_state("bumping", CONVERTER_STATE_BUMPING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::bumped);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::finish_being_bumped);
            efc.change_state("closing");
        }
    }
    
    efc.new_state("closing", CONVERTER_STATE_CLOSING); {
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::open_or_spit);
        }
    }
    
    efc.new_state("spitting", CONVERTER_STATE_SPITTING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::spew);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::open_or_die);
        }
    }
    
    efc.new_state("opening", CONVERTER_STATE_OPENING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::open);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("idling");
        }
    }
    
    efc.new_state("dying", CONVERTER_STATE_DYING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::start_dying);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::finish_dying);
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_CONVERTER_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_CONVERTER_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * Enters the idle state.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::become_idle(mob* m, void* info1, void* info2) {
    converter* c_ptr = (converter*) m;
    
    c_ptr->set_animation(
        c_ptr->get_animation_nr_from_base_and_group(
            CONVERTER_ANIM_IDLING, N_CONVERTER_ANIMS, c_ptr->current_type_nr
        )
    );
    c_ptr->cur_base_anim_nr = CONVERTER_ANIM_IDLING;
    c_ptr->type_change_timer.start();
}


/* ----------------------------------------------------------------------------
 * Does a little bumpy animation after a leader touches it.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::bumped(mob* m, void* info1, void* info2) {
    converter* c_ptr = (converter*) m;
    
    c_ptr->set_animation(
        c_ptr->get_animation_nr_from_base_and_group(
            CONVERTER_ANIM_BUMPING, N_CONVERTER_ANIMS, c_ptr->current_type_nr
        )
    );
    c_ptr->cur_base_anim_nr = CONVERTER_ANIM_BUMPING;
    c_ptr->type_change_timer.stop();
    c_ptr->auto_conversion_timer.stop();
}


/* ----------------------------------------------------------------------------
 * Makes the converter close after it gets bumped.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::finish_being_bumped(mob* m, void* info1, void* info2) {
    ((converter*) m)->close();
}


/* ----------------------------------------------------------------------------
 * Makes the converter vanish.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::finish_dying(mob* m, void* info1, void* info2) {
    m->to_delete = true;
}


/* ----------------------------------------------------------------------------
 * Handles an object bumping against it.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::handle_object_touch(mob* m, void* info1, void* info2) {
    mob* bumper = (mob*) info1;
    if(bumper->type->category->id == MOB_CATEGORY_LEADERS) {
        m->fsm.set_state(CONVERTER_STATE_BUMPING);
    }
}


/* ----------------------------------------------------------------------------
 * Code to handle a Pikmin having been thrown inside.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::handle_pikmin(mob* m, void* info1, void* info2) {
    converter* c_ptr = (converter*) m;
    pikmin* p_ptr = (pikmin*) info1;
    
    if(c_ptr->amount_in_buffer == c_ptr->con_type->buffer_size) {
        //A Pikmin tried to sneak in in the middle of a conversion! Denied.
        return;
    }
    
    c_ptr->amount_in_buffer++;
    if(
        c_ptr->con_type->same_type_counts_for_output ||
        p_ptr->pik_type != c_ptr->current_type
    ) {
        c_ptr->input_pikmin_left--;
    }
    c_ptr->type_change_timer.stop();
    c_ptr->auto_conversion_timer.start();
    
    p_ptr->to_delete = true;
    
    if(
        c_ptr->input_pikmin_left == 0 ||
        c_ptr->amount_in_buffer == c_ptr->con_type->buffer_size
    ) {
        c_ptr->close();
    }
    
    particle p(
        PARTICLE_TYPE_BITMAP, m->pos, m->z + m->height + 1.0,
        24, 1.5, PARTICLE_PRIORITY_MEDIUM
    );
    p.bitmap = game.sys_assets.bmp_smoke;
    particle_generator pg(0, p, 15);
    pg.number_deviation = 5;
    pg.angle = 0;
    pg.angle_deviation = TAU / 2;
    pg.total_speed = 70;
    pg.total_speed_deviation = 10;
    pg.duration_deviation = 0.5;
    pg.emit(game.states.gameplay->particles);
}


/* ----------------------------------------------------------------------------
 * Makes the converter open up.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::open(mob* m, void* info1, void* info2) {
    converter* c_ptr = (converter*) m;
    c_ptr->set_animation(
        c_ptr->get_animation_nr_from_base_and_group(
            CONVERTER_ANIM_OPENING, N_CONVERTER_ANIMS, c_ptr->current_type_nr
        )
    );
    c_ptr->cur_base_anim_nr = CONVERTER_ANIM_OPENING;
}


/* ----------------------------------------------------------------------------
 * Changes to the opening state or the dying state, depending
 * on whether it can still output Pikmin.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::open_or_die(mob* m, void* info1, void* info2) {
    converter* c_ptr = (converter*) m;
    
    if(c_ptr->input_pikmin_left == 0) {
        c_ptr->fsm.set_state(CONVERTER_STATE_DYING);
    } else {
        c_ptr->fsm.set_state(CONVERTER_STATE_OPENING);
    }
}


/* ----------------------------------------------------------------------------
 * Changes to the opening state or the spitting state, depending
 * on whether it has Pikmin in the buffer or not.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::open_or_spit(mob* m, void* info1, void* info2) {
    converter* c_ptr = (converter*) m;
    
    if(c_ptr->amount_in_buffer == 0) {
        c_ptr->fsm.set_state(CONVERTER_STATE_OPENING);
    } else {
        c_ptr->fsm.set_state(CONVERTER_STATE_SPITTING);
    }
}


/* ----------------------------------------------------------------------------
 * Spews out the converted seeds.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::spew(mob* m, void* info1, void* info2) {
    converter* c_ptr = (converter*) m;
    
    c_ptr->set_animation(
        c_ptr->get_animation_nr_from_base_and_group(
            CONVERTER_ANIM_SPITTING, N_CONVERTER_ANIMS, c_ptr->current_type_nr
        )
    );
    c_ptr->cur_base_anim_nr = CONVERTER_ANIM_SPITTING;
    c_ptr->spew();
}


/* ----------------------------------------------------------------------------
 * Makes the converter start dying.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void converter_fsm::start_dying(mob* m, void* info1, void* info2) {
    converter* c_ptr = (converter*) m;
    
    c_ptr->set_animation(
        c_ptr->get_animation_nr_from_base_and_group(
            CONVERTER_ANIM_DYING, N_CONVERTER_ANIMS, c_ptr->current_type_nr
        )
    );
    c_ptr->cur_base_anim_nr = CONVERTER_ANIM_DYING;
}
