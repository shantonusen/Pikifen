/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge type class and bridge type-related functions.
 */

#include "bridge_type.h"

#include "../game.h"
#include "../mob_fsms/bridge_fsm.h"
#include "../mob_script.h"
#include "../mobs/bridge.h"


/* ----------------------------------------------------------------------------
 * Creates a type of bridge.
 */
bridge_type::bridge_type() :
    mob_type(MOB_CATEGORY_BRIDGES),
    bmp_main_texture(nullptr),
    bmp_rail_texture(nullptr) {
    
    radius = 32;
    max_health = 2000;
    pushable = false;
    pushes = false;
    casts_shadow = false;
    can_block_paths = true;
    target_type = MOB_TARGET_TYPE_PIKMIN_OBSTACLE;
    starting_team = MOB_TEAM_OBSTACLE;
    
    area_editor_tips =
        "Place this object on a sector of the \"Bridge\" type.\n"
        "When the bridge object is destroyed, that sector, as well as\n"
        "all neighboring sectors of the \"Bridge\" and \"Bridge rail\" types\n"
        "will be converted into bridges. Their heights will also change to\n"
        "whatever you specify in the sector's \"bridge height\" property.";
        
    bridge_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector bridge_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(BRIDGE_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(BRIDGE_ANIM_DESTROYED, "destroyed"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 * file:
 *   File to read from.
 */
void bridge_type::load_resources(data_node* file) {
    reader_setter rs(file);
    
    rs.set("main_texture", main_texture_file_name);
    rs.set("rail_texture", rail_texture_file_name);
    
    if(!main_texture_file_name.empty()) {
        bmp_main_texture = game.textures.get(main_texture_file_name);
    }
    if(!rail_texture_file_name.empty()) {
        bmp_rail_texture = game.textures.get(rail_texture_file_name);
    }
}


/* ----------------------------------------------------------------------------
 * Unloads resources from memory.
 */
void bridge_type::unload_resources() {
    game.textures.detach(main_texture_file_name);
    game.textures.detach(rail_texture_file_name);
}
