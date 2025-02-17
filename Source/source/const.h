/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally used constants.
 */

#ifndef CONST_INCLUDED
#define CONST_INCLUDED

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "utils/math_utils.h"


using std::size_t;
using std::string;

/*
 * Version stuff.
 * On every release, update these numbers, and
 * update the numbers on the resouce (.rc) file.
 */
const unsigned char VERSION_MAJOR = 0;
const unsigned char VERSION_MINOR = 21;
const unsigned char VERSION_REV   = 1;


//Cross-platform way of representing an invalid index.
constexpr size_t INVALID = UINT32_MAX;
//Cross-platform way of representing a float value of "invalid" or similar.
constexpr float LARGE_FLOAT = 999999.0f;

//Full-white opaque color.
const ALLEGRO_COLOR COLOR_WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
//Full-black opaque color.
const ALLEGRO_COLOR COLOR_BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };
//Fully-transparent color, in black.
const ALLEGRO_COLOR COLOR_EMPTY = { 0.0f, 0.0f, 0.0f, 0.0f };

//The default rotation speed of a mob type.
const float DEF_ROTATION_SPEED = 630.0f;
//The whistle can't go past this radius, by default.
const float DEF_WHISTLE_RANGE = 80.0f;
//How long to suck a mob in for, when being delivered to an Onion/ship.
const float DELIVERY_SUCK_TIME = 2.0f;
//How long to toss a mob in the air for, when being delivered to another mob.
const float DELIVERY_TOSS_TIME = 1.0f;
//If the Pikmin is within this distance of the mob, it can ground attack.
const float GROUNDED_ATTACK_DIST = 5.0f;
//If there's less than this much gap between the leader and group,
//then the group's Pikmin should shuffle a bit to keep up with the leader.
const float GROUP_SHUFFLE_DIST = 40.0f;
//Pikmin must be at least these many units away from one another;
//used when calculating group spots.
const float GROUP_SPOT_INTERVAL = 3.0f;
//The idle glow spins these many radians per second.
const float IDLE_GLOW_SPIN_SPEED = TAU / 4;
//Maximum number of players that can play the game.
const size_t MAX_PLAYERS = 4;
//Multiply the stretch of the shadow by this much.
const float MOB_SHADOW_STRETCH_MULT = 0.5f;
//For every unit above the ground that the mob is on,
//the shadow goes these many units to the side.
const float MOB_SHADOW_Y_MULT = 0.2f;
//When using the height effect, scale the mob by this factor.
const float MOB_HEIGHT_EFFECT_FACTOR = 0.0017;
//Number of maturities.
const size_t N_MATURITIES = 3;
//Name to display on the "none" option in a combobox.
const string NONE_OPTION = "(None)";
//When an opponent is hit, it takes this long to be possible to hit it again.
const float OPPONENT_HIT_REGISTER_TIMEOUT = 0.5f;
//How long a Pikmin that got knocked down stays on the floor for, if left alone.
const float PIKMIN_KNOCKED_DOWN_DURATION = 2.0f;
//A whistled Pikmin that got knocked down loses this much in lie-down time.
const float PIKMIN_KNOCKED_DOWN_WHISTLE_BONUS = 1.3f;
//How frequently should a replay state be saved.
const float REPLAY_SAVE_FREQUENCY = 1.0f;
//Duration of the "smack" particle.
const float SMACK_PARTICLE_DUR = 0.1f;
//The Sun Meter's sun spins these many radians per second.
const float SUN_METER_SUN_SPIN_SPEED = 0.5f;
//Put this space between the leader and the "main" member of the group,
//when using swarming.
const float SWARM_MARGIN = 8.0f;
//When swarming, the group can scale this much vertically.
//Basically, the tube shape's girth can reach this scale.
const float SWARM_VERTICAL_SCALE = 0.5f;
//Tree shadows sway this much away from their neutral position.
const float TREE_SHADOW_SWAY_AMOUNT = 8.0f;
//Tree shadows sway this much per second (TAU = full back-and-forth cycle).
const float TREE_SHADOW_SWAY_SPEED = TAU / 8;
//A new "mob thrown" particle is spawned every X seconds.
const float THROW_PARTICLE_INTERVAL = 0.02f;
//Seconds that need to pass before another dot is added.
const float WHISTLE_DOT_INTERVAL = 0.03;
//A whistle dot spins these many radians a second.
const float WHISTLE_DOT_SPIN_SPEED = TAU / 4;
//Time the whistle animations take to fade out.
const float WHISTLE_FADE_TIME = 0.13f;
//Whistle rings move these many units per second.
const float WHISTLE_RING_SPEED = 600.0f;
//Seconds that need to pass before another whistle ring appears.
const float WHISTLE_RINGS_INTERVAL = 0.1f;

const int SMALLEST_WIN_W = 640;
const int SMALLEST_WIN_H = 480;

const string GAME_DATA_FOLDER_PATH = "Game_data";
const string USER_DATA_FOLDER_PATH = "User_data";
const string TYPES_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Types";
const string MISC_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Misc";
const string TEXTURES_FOLDER_NAME =
    "Textures";
const string USER_AREA_DATA_FOLDER_PATH =
    USER_DATA_FOLDER_PATH + "/Areas";
const string ERROR_LOG_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Error_log.txt";
const string MAKER_TOOLS_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Tools.txt";
const string OPTIONS_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Options.txt";
const string PERFORMANCE_LOG_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Performance_log.txt";

const string ANIMATIONS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Animations";
const string AREAS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Areas";
const string AUDIO_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Audio";
const string GRAPHICS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Graphics";
const string GUI_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/GUI";
const string HAZARDS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Hazards";
const string LIQUIDS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Liquids";
const string PARTICLE_GENERATORS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Particle_generators";
const string SPIKE_DAMAGES_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Spike_damage_types";
const string SPRAYS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Sprays";
const string STATUSES_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Statuses";
const string WEATHER_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Weather";
const string TEXTURES_FOLDER_PATH =
    GRAPHICS_FOLDER_PATH + "/" + TEXTURES_FOLDER_NAME;

const string CONFIG_FILE =
    MISC_FOLDER_PATH + "/Config.txt";
const string SYSTEM_ANIMATIONS_FILE_PATH =
    MISC_FOLDER_PATH + "/System_animations.txt";
const string SYSTEM_ASSET_FILE_NAMES_FILE_PATH =
    MISC_FOLDER_PATH + "/System_asset_file_names.txt";


//List of maker tools.
enum MAKER_TOOLS {
    //None.
    MAKER_TOOL_NONE,
    //Create an image of the whole area.
    MAKER_TOOL_AREA_IMAGE,
    //Change gameplay speed.
    MAKER_TOOL_CHANGE_SPEED,
    //Geometry info beneath mouse cursor.
    MAKER_TOOL_GEOMETRY_INFO,
    //Show hitboxes.
    MAKER_TOOL_HITBOXES,
    //Hurt mob beneath mouse cursor.
    MAKER_TOOL_HURT_MOB,
    //Get info on the mob beneath mouse cursor.
    MAKER_TOOL_MOB_INFO,
    //Create a new Pikmin beneath mouse cursor.
    MAKER_TOOL_NEW_PIKMIN,
    //Teleport to mouse cursor.
    MAKER_TOOL_TELEPORT,
    
    //Total amount of maker tools.
    N_MAKER_TOOLS,
};


const string MAKER_TOOL_NAMES[N_MAKER_TOOLS] = {
    "",
    "area_image",
    "change_speed",
    "geometry_info",
    "hitboxes",
    "hurt_mob",
    "mob_info",
    "new_pikmin",
    "teleport"
};


const unsigned char N_WHISTLE_RING_COLORS = 8;
const unsigned char WHISTLE_RING_COLORS[N_WHISTLE_RING_COLORS][3] = {
    {255, 255, 0  },
    {255, 0,   0  },
    {255, 0,   255},
    {128, 0,   255},
    {0,   0,   255},
    {0,   255, 255},
    {0,   255, 0  },
    {128, 255, 0  }
};
const unsigned char N_WHISTLE_DOT_COLORS = 6;
const unsigned char WHISTLE_DOT_COLORS[N_WHISTLE_DOT_COLORS][3] = {
    {214, 25,  25 }, //Red.
    {242, 134, 48 }, //Orange.
    {143, 227, 58 }, //Lime.
    {55,  222, 222}, //Cyan.
    {30,  30,  219}, //Blue.
    {133, 28,  237}, //Purple.
};


#endif //ifndef CONST_INCLUDED
