/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion type class and Onion type-related functions.
 */

#ifndef ONION_TYPE_INCLUDED
#define ONION_TYPE_INCLUDED

#include "../utils/data_file.h"
#include "../mobs/mob_utils.h"
#include "mob_type.h"
#include "pikmin_type.h"


//Onion object states.
enum ONION_STATES {
    //Idling.
    ONION_STATE_IDLING,
    
    //Total amount of Onion object states.
    N_ONION_STATES,
};


const float ONION_FULL_SPEW_DELAY          = 2.5f;
const float ONION_NEXT_SPEW_DELAY          = 0.10f;
const unsigned char ONION_SEETHROUGH_ALPHA = 128;
const float ONION_FADE_SPEED               = 255; //Values per second.


/* ----------------------------------------------------------------------------
 * An Onion type.
 * It's basically associated with one or more Pikmin types.
 */
class onion_type : public mob_type {
public:
    onion_type();
    ~onion_type();
    
    //Nest data.
    pikmin_nest_type_struct* nest;
    
    void load_properties(data_node* file);
    void load_resources(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
};


#endif //ifndef ONION_TYPE_INCLUDED
