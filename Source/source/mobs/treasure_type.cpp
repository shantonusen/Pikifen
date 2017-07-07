/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure type class and treasure type-related functions.
 */

#include "../functions.h"
#include "mob_fsm.h"
#include "treasure.h"
#include "treasure_fsm.h"
#include "treasure_type.h"

/* ----------------------------------------------------------------------------
 * Creates a type of treasure.
 */
treasure_type::treasure_type() :
    mob_type(MOB_CATEGORY_TREASURES),
    value(0) {
    
    treasure_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void treasure_type::load_parameters(data_node* file) {
    value = s2f(file->get_child_by_name("value")->value);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector treasure_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(ANIM_IDLING, "idling"));
    return v;
}