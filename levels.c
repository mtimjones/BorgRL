// levels.c
#include "ces.h"

static int level = 0;

// levels defines the parameters used to set each level of the game (including 
// settings for procedural generation).

typedef struct
{
    cdrone_type_t type;                         // TP
    unsigned int  combat_drones;                // CD
    unsigned int  cd_attack;                    // CDT
    unsigned int  cd_armor;                     // CDR
    unsigned int  cd_hp;                        // CDH
    unsigned int  cd_attack_speed;              // CAS
    unsigned int  cd_move_speed;                // CMS
} drone_t;

typedef struct
{
    unsigned int wrecks;                       // W
    unsigned int wreck_resources;              // WR

    unsigned int planets;                      // P
    unsigned int planet_resources;             // PR

    unsigned int academies;                    // A

    unsigned int gas_cloud_rays;               // GCR
    unsigned int gas_cloud_ray_len;            // GCL
    unsigned int gas_smoothing_param;          // GSP
    unsigned int gas_smoothing_iters;          // GSI
    unsigned int gas_smoothing_filler_param;   // GSF

    drone_t  drones[ MAX_DRONE_TYPES ];

} levels_t;

const levels_t levels[] = {
//    W   WR  P   PR   A  GCR GCL GSP GSI GSF
//      Type    CD CDT CDR CDH CAS CMS 
    { 10,  5,  0,  0,  1,  20, 80,  4, 10,  7,
    { { Nova,    6,  1,  1,  2, 60, 10 },
      { Raven,   3,  1,  2,  2, 55,  9 },
      { Hunter,  0,  0,  0,  0,  0,  8 },
      { Kestral, 0,  0,  0,  0,  0,  7 },
      { Tank,    0,  0,  0,  0,  0,  9 } },
    }, // 0

    {  4,  7,  3,  8,  1,  30, 60,  3, 10,  8,
    { { Nova,    2,  1,  2,  3, 60, 10 }, 
      { Raven,   4,  1,  2,  2, 55,  9 }, 
      { Hunter,  2,  2,  2,  3, 50,  8 },
      { Kestral, 0,  0,  0,  0,  0,  7 },
      { Tank ,   0,  0,  0,  0,  0,  9 } }, 
    }, // 1

    {  6,  8,  6, 10,  1,  10, 40,  3, 10,  6,
    { { Nova,    0,  0,  0,  0,  0,  0 }, 
      { Raven,   3,  2,  2,  4, 55,  9 }, 
      { Hunter,  2,  3,  2,  4, 50,  8 },
      { Kestral, 2,  3,  2,  5, 45,  7 },
      { Tank ,   0,  0,  0,  0,  0,  0 } }, 
    }, // 2

    {  6, 10,  5,  9,  1,  10, 40,  3, 10,  6,
    { { Nova,    1,  2,  2,  3, 60, 10 }, 
      { Raven,   1,  3,  2,  4, 55,  9 }, 
      { Hunter,  2,  3,  3,  5, 50,  8 },
      { Kestral, 2,  3,  2,  5, 45,  7 },
      { Tank ,   2,  3,  4,  6, 70,  9 } }, 
    }  // 3
};

// Accessors for the level and max-level.

int get_level( void )
{
    return level;
}

void inc_level( void )
{
    level++;
}

int get_max_level( void )
{
    return sizeof(levels)/sizeof(levels_t) - 1;
}

// Accessors for the levels data.

unsigned int get_wreck_count( int level )
{
    return levels[ level ].wrecks;
}

unsigned int get_wreck_resources( int level )
{
    return levels[ level ].wreck_resources;
}

unsigned int get_academies( int level )
{
    return levels[ level ].academies;
}

unsigned int get_planet_count( int level )
{
    return levels[ level ].planets;
}

unsigned int get_planet_resources( int level )
{
    return levels[ level ].planet_resources;
}

cdrone_type_t get_cdrone_type( int level, int index )
{
    return levels[ level ].drones[ index ].type;
}

unsigned int get_combat_drones( int level, int index )
{
    return levels[ level ].drones[ index ].combat_drones;
} 

unsigned int get_cd_attack( int level, int index )
{
    return levels[ level ].drones[ index ].cd_attack;
} 

unsigned int get_cd_armor( int level, int index )
{
    return levels[ level ].drones[ index ].cd_armor;
} 

unsigned int get_cd_hp( int level, int index )
{
    return levels[ level ].drones[ index ].cd_hp;
} 

unsigned int get_cd_attack_speed( int level, int index )
{
    return levels[ level ].drones[ index ].cd_attack_speed;
} 

unsigned int get_cd_move_speed( int level, int index )
{
    return levels[ level ].drones[ index ].cd_move_speed;
} 

unsigned int get_gas_cloud_rays( int level )
{
    return levels[ level ].gas_cloud_rays;
}

unsigned int get_gas_cloud_ray_len( int level )
{
    return levels[ level ].gas_cloud_ray_len;
}

unsigned int get_gas_smoothing_param( int level )
{
    return levels[ level ].gas_smoothing_param;
}

unsigned int get_gas_smoothing_filler_param( int level )
{
    return levels[ level ].gas_smoothing_filler_param;
}

unsigned int get_gas_smoothing_iters( int level )
{
    return levels[ level ].gas_smoothing_iters;
}

