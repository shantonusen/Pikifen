/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge class and bridge-related functions.
 */

#ifndef BRIDGE_INCLUDED
#define BRIDGE_INCLUDED

#include "../mob_types/bridge_type.h"
#include "mob.h"


/* ----------------------------------------------------------------------------
 * A bridge mob. Bridges on the engine are made up of two parts:
 * the mob itself, which Pikmin damage, and a series of components.
 * Each component is a mob that other mobs can walk on top of, serving
 * either as the floor of the bridge, or one of the railings.
 * Every time the bridge expands, it is considered that a new chunk has
 * been added, which may either generate new components, or stretch the
 * existing ones.
 */
class bridge : public mob {
private:
    //How many chunks are needed to fully build this bridge.
    size_t total_chunks_needed;
    //How many chunks have successfully been created so far.
    size_t chunks;
    
public:
    //What type of bridge it is.
    bridge_type* bri_type;
    
    //Sectors it will affect when it opens.
    vector<sector*> secs;
    
    //Constructor.
    bridge(const point &pos, bridge_type* bri_type, const float angle);
    
    //Draws a bridge component.
    static void draw_component(mob* m);
    //Checks if any chunks need to be created, and creates them if needed.
    void check_health();
};


#endif //ifndef BRIDGE_INCLUDED
