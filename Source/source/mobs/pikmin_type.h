/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin type class and Pikmin type-related functions.
 */

#ifndef PIKMIN_TYPE_INCLUDED
#define PIKMIN_TYPE_INCLUDED

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "../data_file.h"
#include "../hazard.h"
#include "mob_type.h"

using namespace std;

class leader;

/* ----------------------------------------------------------------------------
 * Pikmin types, almost the basic meat of the fan-games.
 * The canon ones are Red, Yellow, Blue, White,
 * Purple, Bulbmin, Winged and Rock, but with the engine,
 * loads of fan-made ones can be made.
 */
class pikmin_type : public mob_type {
public:
    vector<hazard*> resistances;
    float attack_power;
    float attack_interval;
    float carry_strength;
    float carry_speed;
    float throw_height_mult;
    bool has_onion;
    bool can_dig;
    bool can_fly;
    bool can_swim;
    bool can_latch;
    bool can_carry_bomb_rocks;
    //Top (leaf/bud/flower) bitmap for each maturity.
    ALLEGRO_BITMAP* bmp_top[3];
    //Standby icons for each maturity.
    ALLEGRO_BITMAP* bmp_icon[3];

    pikmin_type();
    void load_from_file(
        data_node* file, const bool load_resources,
        vector<pair<size_t, string> >* anim_conversions
    );

};

#endif //ifndef PIKMIN_TYPE_INCLUDED
