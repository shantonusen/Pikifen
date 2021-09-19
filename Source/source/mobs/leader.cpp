/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader class and leader-related functions.
 */

#include <algorithm>

#include "leader.h"

#include "../const.h"
#include "../drawing.h"
#include "../functions.h"
#include "../game.h"


const float leader::THROW_COOLDOWN_DURATION = 0.15f;
const float leader::AUTO_THROW_COOLDOWN_MAX_DURATION = 0.7f;
const float leader::AUTO_THROW_COOLDOWN_MIN_DURATION =
    THROW_COOLDOWN_DURATION * 1.2f;
const float leader::AUTO_THROW_COOLDOWN_SPEED = 0.3f;


/* ----------------------------------------------------------------------------
 * Creates a leader mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Leader type this mob belongs to.
 * angle:
 *   Starting angle.
 */
leader::leader(const point &pos, leader_type* type, const float angle) :
    mob(pos, type, angle),
    lea_type(type),
    active(false),
    auto_plucking(false),
    pluck_target(nullptr),
    queued_pluck_cancel(false),
    is_in_walking_anim(false),
    throw_cooldown(0.0f),
    throw_queued(false),
    auto_throwing(false),
    auto_throw_cooldown(0.0f),
    throwee(nullptr),
    throwee_angle(0.0f),
    throwee_max_z(0.0f),
    throwee_speed_z(0.0f),
    throwee_can_reach(false) {
    
    team = MOB_TEAM_PLAYER_1;
    invuln_period = timer(LEADER_INVULN_PERIOD);
    
    subgroup_type_ptr =
        game.states.gameplay->subgroup_types.get_type(
            SUBGROUP_TYPE_CATEGORY_LEADER
        );
}


/* ----------------------------------------------------------------------------
 * Returns whether or not a leader can receive a given status effect.
 * s:
 *   Status type to check.
 */
bool leader::can_receive_status(status_type* s) const {
    return s->affects & STATUS_AFFECTS_LEADERS;
}


/* ----------------------------------------------------------------------------
 * Returns whether or not a leader can throw.
 */
bool leader::check_throw_ok() const {
    if(holding.empty()) {
        return false;
    }
    
    mob_event* ev = fsm.get_event(LEADER_EV_THROW);
    
    if(!ev) {
        return false;
    }
    
    return true;
}


//Members cannot go past this range from the angle of dismissal.
const float DISMISS_ANGLE_RANGE = TAU / 2;
//Multiply the space members take up by this. Lower = more compact subgroups.
const float DISMISS_MEMBER_SIZE_MULTIPLIER = 0.75f;
//Dismissed groups must have this much distance between them/the leader.
const float DISMISS_SUBGROUP_DISTANCE = 48.0f;

/* ----------------------------------------------------------------------------
 * Makes a leader dismiss their group.
 * The group is then organized in groups, by type,
 * and is dismissed close to the leader.
 */
void leader::dismiss() {
    size_t n_group_members = group->members.size();
    if(n_group_members == 0) return;
    
    //They are dismissed towards this angle.
    //This is then offset a bit for each subgroup, depending on a few factors.
    float base_angle;
    
    //First, calculate what direction the group should be dismissed to.
    if(game.states.gameplay->swarm_magnitude > 0) {
        //If the leader's swarming,
        //they should be dismissed in that direction.
        base_angle = game.states.gameplay->swarm_angle;
    } else {
        //Leftmost member coordinate, rightmost, etc.
        point min_coords, max_coords;
        
        for(size_t m = 0; m < n_group_members; ++m) {
            mob* member_ptr = group->members[m];
            
            if(member_ptr->pos.x < min_coords.x || m == 0)
                min_coords.x = member_ptr->pos.x;
            if(member_ptr->pos.x > max_coords.x || m == 0)
                max_coords.x = member_ptr->pos.x;
            if(member_ptr->pos.y < min_coords.y || m == 0)
                min_coords.y = member_ptr->pos.y;
            if(member_ptr->pos.y > max_coords.y || m == 0)
                max_coords.y = member_ptr->pos.y;
        }
        
        point group_center(
            (min_coords.x + max_coords.x) / 2,
            (min_coords.y + max_coords.y) / 2
        );
        base_angle = get_angle(pos, group_center);
    }
    
    
    struct subgroup_dismiss_info {
        subgroup_type* type;
        mob_type* m_type;
        float radius;
        vector<mob*> members;
        size_t row;
        point center;
    };
    vector<subgroup_dismiss_info> subgroups_info;
    
    //Go through all subgroups and populate the vector of data.
    subgroup_type* first_type =
        game.states.gameplay->subgroup_types.get_first_type();
    subgroup_type* cur_type = first_type;
    
    do {
    
        if(
            cur_type !=
            game.states.gameplay->subgroup_types.get_type(
                SUBGROUP_TYPE_CATEGORY_LEADER
            )
        ) {
        
            bool subgroup_exists = false;
            
            for(size_t m = 0; m < n_group_members; ++m) {
                mob* m_ptr = group->members[m];
                if(m_ptr->subgroup_type_ptr != cur_type) continue;
                
                if(!subgroup_exists) {
                    subgroups_info.push_back(subgroup_dismiss_info());
                    subgroups_info.back().m_type = m_ptr->type;
                    subgroup_exists = true;
                }
                
                subgroups_info.back().members.push_back(m_ptr);
            }
            
        }
        
        cur_type =
            game.states.gameplay->subgroup_types.get_next_type(cur_type);
            
    } while(cur_type != first_type);
    
    //Let's figure out each subgroup's size.
    //Subgroups will be made by placing the members in
    //rows of circles surrounding a central point.
    //The first row is just one spot.
    //The second row is 6 spots around that one.
    //The third is 12 spots around those 6.
    //And so on. Each row fits an additional 6.
    for(size_t s = 0; s < subgroups_info.size(); ++s) {
        size_t n_rows = get_dismiss_rows(subgroups_info[s].members.size());
        
        //Since each row loops all around,
        //it appears to the left and right of the center.
        //So count each one twice. Except for the central one.
        subgroups_info[s].radius =
            game.config.standard_pikmin_radius +
            game.config.standard_pikmin_radius * 2 *
            DISMISS_MEMBER_SIZE_MULTIPLIER * (n_rows - 1);
    }
    
    //We'll need to place the subgroups inside arched rows.
    //Like stripes on a rainbow.
    //For each row, we must fit as many Pikmin subgroups as possible.
    //Each row can have a different thickness,
    //based on the size of the subgroups within.
    //Starts off on the row closest to the leader.
    //We place the first subgroup, then some padding, then the next group,
    //etc. For every subgroup we place, we must update the thickness.
    struct row_info {
        vector<size_t> subgroups;
        float dist_between_center;
        float thickness;
        float angle_occupation; //How much is taken up by Pikmin and padding.
        
        row_info() {
            dist_between_center = 0;
            thickness = 0;
            angle_occupation = 0;
        }
    };
    
    bool done = false;
    vector<row_info> rows;
    row_info cur_row;
    cur_row.dist_between_center = DISMISS_SUBGROUP_DISTANCE;
    size_t cur_row_nr = 0;
    size_t cur_subgroup_nr = 0;
    
    while(!done && !subgroups_info.empty()) {
        float new_thickness =
            std::max(
                cur_row.thickness, subgroups_info[cur_subgroup_nr].radius * 2
            );
            
        float new_angle_occupation = 0;
        for(size_t s = 0; s < cur_row.subgroups.size(); ++s) {
            new_angle_occupation +=
                linear_dist_to_angular(
                    subgroups_info[cur_row.subgroups[s]].radius * 2.0,
                    cur_row.dist_between_center +
                    cur_row.thickness / 2.0f
                );
            if(s < cur_row.subgroups.size() - 1) {
                new_angle_occupation +=
                    linear_dist_to_angular(
                        DISMISS_SUBGROUP_DISTANCE,
                        cur_row.dist_between_center +
                        cur_row.thickness / 2.0f
                    );
            }
        }
        if(!cur_row.subgroups.empty()) {
            new_angle_occupation +=
                linear_dist_to_angular(
                    DISMISS_SUBGROUP_DISTANCE,
                    cur_row.dist_between_center +
                    new_thickness / 2.0f
                );
        }
        new_angle_occupation +=
            linear_dist_to_angular(
                subgroups_info[cur_subgroup_nr].radius * 2.0,
                cur_row.dist_between_center +
                new_thickness / 2.0f
            );
            
        //Will this group fit?
        if(new_angle_occupation <= DISMISS_ANGLE_RANGE) {
            //This subgroup still fits. Next!
            cur_row.thickness = new_thickness;
            cur_row.angle_occupation = new_angle_occupation;
            
            cur_row.subgroups.push_back(cur_subgroup_nr);
            subgroups_info[cur_subgroup_nr].row = cur_row_nr;
            cur_subgroup_nr++;
        }
        
        if(
            new_angle_occupation > DISMISS_ANGLE_RANGE ||
            cur_subgroup_nr == subgroups_info.size()
        ) {
            //This subgroup doesn't fit. It'll have to be put in the next row.
            //Or this is the last subgroup, and the row needs to be committed.
            
            rows.push_back(cur_row);
            cur_row_nr++;
            cur_row.dist_between_center +=
                cur_row.thickness + DISMISS_SUBGROUP_DISTANCE;
            cur_row.subgroups.clear();
            cur_row.thickness = 0;
            cur_row.angle_occupation = 0;
        }
        
        if(cur_subgroup_nr == subgroups_info.size()) done = true;
    }
    
    //Now that we know which subgroups go into which row,
    //simply decide the positioning.
    for(size_t r = 0; r < rows.size(); ++r) {
        float start_angle = -(rows[r].angle_occupation / 2.0f);
        float cur_angle = start_angle;
        
        for(size_t s = 0; s < rows[r].subgroups.size(); ++s) {
            size_t s_nr = rows[r].subgroups[s];
            float subgroup_angle = cur_angle;
            
            cur_angle +=
                linear_dist_to_angular(
                    subgroups_info[s_nr].radius * 2.0,
                    rows[r].dist_between_center + rows[r].thickness / 2.0
                );
            if(s < rows[r].subgroups.size() - 1) {
                cur_angle +=
                    linear_dist_to_angular(
                        DISMISS_SUBGROUP_DISTANCE,
                        rows[r].dist_between_center + rows[r].thickness / 2.0
                    );
            }
            
            //Center the subgroup's angle.
            subgroup_angle +=
                linear_dist_to_angular(
                    subgroups_info[s_nr].radius,
                    rows[r].dist_between_center + rows[r].thickness / 2.0
                );
                
            subgroups_info[s_nr].center =
                angle_to_coordinates(
                    base_angle + subgroup_angle,
                    rows[r].dist_between_center + rows[r].thickness / 2.0f
                );
                
        }
    }
    
    //Now, dismiss!
    for(size_t s = 0; s < subgroups_info.size(); ++s) {
        cur_row_nr = 0;
        size_t cur_row_spot_nr = 0;
        size_t cur_row_spots = 1;
        
        for(size_t m = 0; m < subgroups_info[s].members.size(); ++m) {
        
            point destination;
            
            if(cur_row_nr == 0) {
                destination = subgroups_info[s].center;
            } else {
                float member_angle =
                    ((float) cur_row_spot_nr / cur_row_spots) * TAU;
                destination =
                    subgroups_info[s].center +
                    angle_to_coordinates(
                        member_angle,
                        cur_row_nr * game.config.standard_pikmin_radius * 2 *
                        DISMISS_MEMBER_SIZE_MULTIPLIER
                    );
            }
            
            destination +=
                point(
                    randomf(-5.0, 5.0),
                    randomf(-5.0, 5.0)
                );
                
            cur_row_spot_nr++;
            if(cur_row_spot_nr == cur_row_spots) {
                cur_row_nr++;
                cur_row_spot_nr = 0;
                if(cur_row_nr == 1) {
                    cur_row_spots = 6;
                } else {
                    cur_row_spots += 6;
                }
            }
            
            destination += this->pos;
            
            subgroups_info[s].members[m]->leave_group();
            subgroups_info[s].members[m]->fsm.run_event(
                MOB_EV_DISMISSED, (void*) &destination
            );
            
        }
    }
    
    //Dismiss leaders now.
    while(!group->members.empty()) {
        group->members[0]->fsm.run_event(MOB_EV_DISMISSED, NULL);
        group->members[0]->leave_group();
    }
    
    //Final things.
    lea_type->sfx_dismiss.play(0, false);
    set_animation(LEADER_ANIM_DISMISSING);
}


/* ----------------------------------------------------------------------------
 * Draw a leader mob.
 */
void leader::draw_mob() {
    mob::draw_mob();
    
    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(s_ptr, &eff, true, true);
    
    if(invuln_period.time_left > 0.0f) {
        sprite* spark_s =
            game.sys_assets.spark_animation.instance.get_cur_sprite();
            
        if(spark_s && spark_s->bitmap) {
            bitmap_effect_info spark_eff = eff;
            point size(
                al_get_bitmap_width(s_ptr->bitmap) * eff.scale.x,
                al_get_bitmap_height(s_ptr->bitmap) * eff.scale.y
            );
            spark_eff.scale.x = size.x / al_get_bitmap_width(spark_s->bitmap);
            spark_eff.scale.y = size.y / al_get_bitmap_height(spark_s->bitmap);
            draw_bitmap_with_effects(spark_s->bitmap, spark_eff);
        }
    }
    
    draw_status_effect_bmp(this, eff);
}


/* ----------------------------------------------------------------------------
 * Returns how many rows will be needed to fit all of the members.
 * Used to calculate how subgroup members will be placed when dismissing.
 * n_members:
 *   Total number of group members to dismiss.
 */
size_t leader::get_dismiss_rows(const size_t n_members) const {
    size_t members_that_fit = 1;
    size_t rows_needed = 1;
    while(members_that_fit < n_members) {
        rows_needed++;
        members_that_fit += 6 * (rows_needed - 1);
    }
    return rows_needed;
}


/* ----------------------------------------------------------------------------
 * Returns its group spot information.
 * Basically, when it's in a leader's group, what point it should be following,
 * and within what distance.
 * final_spot:
 *   The final coordinates are returned here.
 * final_dist:
 *   The final distance to those coordinates is returned here.
 */
void leader::get_group_spot_info(
    point* final_spot, float* final_dist
) const {
    final_spot->x = 0.0f;
    final_spot->y = 0.0f;
    *final_dist = 0.0f;
    
    if(!following_group || !following_group->group) {
        return;
    }
    
    group_info_struct* leader_group_ptr = following_group->group;
    
    float distance =
        following_group->radius +
        radius + game.config.standard_pikmin_radius;
        
    for(size_t me = 0; me < leader_group_ptr->members.size(); ++me) {
        mob* member_ptr = leader_group_ptr->members[me];
        if(member_ptr == this) {
            break;
        } else if(member_ptr->subgroup_type_ptr == subgroup_type_ptr) {
            //If this member is also a leader,
            //then that means the current leader should stick behind.
            distance +=
                member_ptr->radius * 2 + GROUP_SPOT_INTERVAL;
        }
    }
    
    *final_spot = following_group->pos;
    *final_dist = distance;
}


/* ----------------------------------------------------------------------------
 * Orders Pikmin from the group to leave the group, and head for the specified
 * nest, with the goal of being stored inside. This function prioritizes
 * less matured Pikmin, and ones closest to the nest.
 * Returns true if the specified number of Pikmin were successfully ordered,
 * and false if there were not enough Pikmin of that type in the group
 * to fulfill the order entirely.
 * type:
 *   Type of Pikmin to order.
 * n_ptr:
 *   Nest to enter.
 * amount:
 *   Amount of Pikmin of the given type to order.
 */
bool leader::order_pikmin_to_onion(
    pikmin_type* type, pikmin_nest_struct* n_ptr, const size_t amount
) {
    //Find Pikmin of that type.
    vector<std::pair<dist, pikmin*>> candidates;
    size_t amount_ordered = 0;
    
    for(size_t m = 0; m < group->members.size(); ++m) {
        mob* mob_ptr = group->members[m];
        if(
            mob_ptr->type->category->id != MOB_CATEGORY_PIKMIN ||
            mob_ptr->type != type
        ) {
            continue;
        }
        
        candidates.push_back(
            std::make_pair(
                dist(mob_ptr->pos, n_ptr->m_ptr->pos),
                (pikmin*) mob_ptr
            )
        );
    }
    
    //Sort them by maturity first, distance second.
    std::sort(
        candidates.begin(),
        candidates.end(),
        [] (
            std::pair<dist, pikmin*> &p1,
            std::pair<dist, pikmin*> &p2
    ) -> bool {
        if(p1.second->maturity != p2.second->maturity) {
            return p1.second->maturity < p2.second->maturity;
        } else {
            return p1.first < p2.first;
        }
    }
    );
    
    //Order Pikmin, in order.
    for(size_t p = 0; p < candidates.size(); ++p) {
    
        pikmin* pik_ptr = candidates[p].second;
        mob_event* ev = pik_ptr->fsm.get_event(MOB_EV_GO_TO_ONION);
        if(!ev) continue;
        
        ev->run(pik_ptr, (void*) n_ptr);
        
        amount_ordered++;
        if(amount_ordered == amount) {
            return true;
        }
    }
    
    //If it got here, that means we couldn't order enough Pikmin to fulfill
    //the requested amount.
    return false;
}


/* ----------------------------------------------------------------------------
 * Signals the group members that the swarm mode stopped.
 */
void leader::signal_swarm_end() const {
    for(size_t m = 0; m < group->members.size(); ++m) {
        group->members[m]->fsm.run_event(MOB_EV_SWARM_ENDED);
    }
}


/* ----------------------------------------------------------------------------
 * Signals the group members that the swarm mode started.
 */
void leader::signal_swarm_start() const {
    for(size_t m = 0; m < group->members.size(); ++m) {
        group->members[m]->fsm.run_event(MOB_EV_SWARM_STARTED);
    }
}


/* ----------------------------------------------------------------------------
 * Starts the auto-throw mode.
 */
void leader::start_auto_throwing() {
    auto_throwing = true;
    auto_throw_cooldown = 0.0f;
    auto_throw_cooldown_duration = AUTO_THROW_COOLDOWN_MAX_DURATION;
}


/* ----------------------------------------------------------------------------
 * Starts the particle generator that leaves a trail behind a thrown Pikmin.
 */
void leader::start_throw_trail() {
    particle throw_p(
        PARTICLE_TYPE_CIRCLE, pos, z,
        radius, 0.6, PARTICLE_PRIORITY_LOW
    );
    throw_p.size_grow_speed = -5;
    throw_p.color = change_alpha(type->main_color, 128);
    particle_generator pg(THROW_PARTICLE_INTERVAL, throw_p, 1);
    pg.follow_mob = this;
    pg.id = MOB_PARTICLE_GENERATOR_THROW;
    particle_generators.push_back(pg);
}


/* ----------------------------------------------------------------------------
 * Makes the leader start whistling.
 */
void leader::start_whistling() {
    game.states.gameplay->whistle.start_whistling();
    lea_type->sfx_whistle.play(0, false);
    set_animation(LEADER_ANIM_WHISTLING);
    script_timer.start(2.5f);
}


/* ----------------------------------------------------------------------------
 * Stops the auto-throw mode.
 */
void leader::stop_auto_throwing() {
    auto_throwing = false;
}


/* ----------------------------------------------------------------------------
 * Makes the leader stop whistling.
 */
void leader::stop_whistling() {
    if(!game.states.gameplay->whistle.whistling) return;
    game.states.gameplay->whistle.stop_whistling();
    lea_type->sfx_whistle.stop();
}


/* ----------------------------------------------------------------------------
 * Swaps out the currently held Pikmin for a different one.
 * new_pik:
 *   The new Pikmin to hold.
 */
void leader::swap_held_pikmin(mob* new_pik) {
    if(holding.empty()) return;
    
    mob_event* old_pik_ev = holding[0]->fsm.get_event(MOB_EV_RELEASED);
    mob_event* new_pik_ev = new_pik->fsm.get_event(MOB_EV_GRABBED_BY_FRIEND);
    
    group->sort(new_pik->subgroup_type_ptr);
    
    if(!old_pik_ev || !new_pik_ev) return;
    
    new_pik_ev->run(new_pik);
    
    release(holding[0]);
    hold(
        new_pik, INVALID, LEADER_HELD_MOB_DIST, LEADER_HELD_MOB_ANGLE,
        false, true
    );
    
    game.sys_assets.sfx_switch_pikmin.play(0, false);
}


/* ----------------------------------------------------------------------------
 * Ticks leader-related logic for this frame.
 * delta_t:
 *   How many seconds to tick by.
 */
void leader::tick_class_specifics(const float delta_t) {
    //Throw-related things.
    update_throw_variables();
    
    if(auto_throw_cooldown > 0.0f) {
        auto_throw_cooldown -= delta_t;
    }
    if(throw_cooldown > 0.0f) {
        throw_cooldown -= delta_t;
    }
    
    if(auto_throwing && auto_throw_cooldown <= 0.0f) {
        bool grabbed = grab_closest_group_member();
        if(grabbed) {
            queue_throw();
        }
        auto_throw_cooldown = auto_throw_cooldown_duration;
    }
    
    if(
        throw_queued &&
        throw_cooldown <= 0.0f &&
        check_throw_ok()
    ) {
        fsm.run_event(LEADER_EV_THROW);
        update_throw_variables();
        throw_cooldown = THROW_COOLDOWN_DURATION;
        throw_queued = false;
    }
    
    if(throw_cooldown <= 0.0f) {
        throw_queued = false;
    }
    
    auto_throw_cooldown_duration =
        std::max(
            auto_throw_cooldown_duration - AUTO_THROW_COOLDOWN_SPEED * delta_t,
            AUTO_THROW_COOLDOWN_MIN_DURATION
        );
        
    if(group && group->members.empty()) {
        stop_auto_throwing();
    }
}


/* ----------------------------------------------------------------------------
 * Queues up a throw. This will cause the throw to go through whenever
 * the throw cooldown ends.
 */
void leader::queue_throw() {
    if(!check_throw_ok()) {
        return;
    }
    
    throw_queued = true;
}


/* ----------------------------------------------------------------------------
 * Updates variables related to how the leader's throw would go.
 */
void leader::update_throw_variables() {
    throwee = NULL;
    if(!holding.empty()) {
        throwee = holding[0];
    } else if(game.states.gameplay->cur_leader_ptr == this) {
        throwee = game.states.gameplay->closest_group_member;
    }
    
    if(!throwee) {
        return;
    }
    
    float target_z;
    if(game.states.gameplay->throw_dest_mob) {
        target_z =
            game.states.gameplay->throw_dest_mob->z +
            game.states.gameplay->throw_dest_mob->height;
    } else if(game.states.gameplay->throw_dest_sector) {
        target_z = game.states.gameplay->throw_dest_sector->z;
    } else {
        target_z = z;
    }
    
    float max_height;
    switch (throwee->type->category->id) {
    case MOB_CATEGORY_PIKMIN: {
        max_height = ((pikmin*) throwee)->pik_type->max_throw_height;
        break;
    } case MOB_CATEGORY_LEADERS: {
        max_height = ((leader*) throwee)->lea_type->max_throw_height;
        break;
    } default: {
        max_height = std::max(128.0f, (target_z - z) * 1.2f);
        break;
    }
    }
    
    if(max_height >= (target_z - z)) {
        //Can reach.
        throwee_can_reach = true;
    } else {
        //Can't reach! Just do a convincing throw that is sure to fail.
        //Limiting the "target" Z makes it so the horizontal velocity isn't
        //so wild.
        target_z = z + max_height * 0.75;
        throwee_can_reach = false;
    }
    
    throwee_max_z = z + max_height;
    
    calculate_throw(
        pos,
        z,
        game.states.gameplay->throw_dest,
        target_z,
        max_height,
        GRAVITY_ADDER,
        &throwee_speed,
        &throwee_speed_z,
        &throwee_angle
    );
}


/* ----------------------------------------------------------------------------
 * Switch active leader.
 * forward:
 *   If true, switch to the next one. If false, to the previous.
 * force_success:
 *   If true, switch to this leader even if they can't currently handle the
 *   leader switch script event.
 */
void change_to_next_leader(const bool forward, const bool force_success) {
    if(game.states.gameplay->mobs.leaders.size() == 1) return;
    
    if(
        !game.states.gameplay->cur_leader_ptr->fsm.get_event(
            LEADER_EV_INACTIVATED
        ) && !force_success
    ) {
        //This leader isn't ready to be switched out of. Forget it.
        return;
    }
    
    //We'll send the switch event to the next leader on the list.
    //If they accept, they run a function to change leaders.
    //If not, we try the next leader.
    //If we return to the current leader without anything being
    //changed, then stop trying; no leader can be switched to.
    
    size_t new_leader_nr = game.states.gameplay->cur_leader_nr;
    leader* new_leader_ptr = NULL;
    bool searching = true;
    size_t original_leader_nr = game.states.gameplay->cur_leader_nr;
    bool cant_find_new_leader = false;
    
    while(searching) {
        new_leader_nr =
            sum_and_wrap(
                new_leader_nr,
                (forward ? 1 : -1),
                game.states.gameplay->mobs.leaders.size()
            );
        new_leader_ptr = game.states.gameplay->mobs.leaders[new_leader_nr];
        
        if(new_leader_nr == original_leader_nr) {
            //Back to the original; stop trying.
            cant_find_new_leader = true;
            searching = false;
        }
        
        new_leader_ptr->fsm.run_event(LEADER_EV_ACTIVATED);
        
        //If after we called the event, the leader is the same,
        //then that means the leader can't be switched to.
        //Try a new one.
        if(game.states.gameplay->cur_leader_nr != original_leader_nr) {
            searching = false;
        }
    }
    
    if(cant_find_new_leader && force_success) {
        //Ok, we need to force a leader to accept the focus. Let's do so.
        game.states.gameplay->cur_leader_nr =
            sum_and_wrap(
                new_leader_nr,
                (forward ? 1 : -1),
                game.states.gameplay->mobs.leaders.size()
            );
        game.states.gameplay->cur_leader_ptr =
            game.states.gameplay->
            mobs.leaders[game.states.gameplay->cur_leader_nr];
            
        game.states.gameplay->cur_leader_ptr->fsm.set_state(
            LEADER_STATE_ACTIVE
        );
    }
}


/* ----------------------------------------------------------------------------
 * Makes the current leader grab the closest group member of the standby type.
 * Returns true on success, false on failure.
 */
bool grab_closest_group_member() {
    if(game.states.gameplay->closest_group_member) {
        mob_event* grabbed_ev =
            game.states.gameplay->closest_group_member->fsm.get_event(
                MOB_EV_GRABBED_BY_FRIEND
            );
        mob_event* grabber_ev =
            game.states.gameplay->cur_leader_ptr->fsm.get_event(
                LEADER_EV_HOLDING
            );
        if(grabber_ev && grabbed_ev) {
            game.states.gameplay->cur_leader_ptr->fsm.run_event(
                LEADER_EV_HOLDING,
                (void*) game.states.gameplay->closest_group_member
            );
            grabbed_ev->run(
                game.states.gameplay->closest_group_member,
                (void*) game.states.gameplay->closest_group_member
            );
            return true;
        }
    }
    return false;
}
