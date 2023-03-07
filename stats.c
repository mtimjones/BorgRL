// stats.c

#include <strings.h>
#include "stats.h"

typedef struct
{
    unsigned int drones_assimilated;
    unsigned int enemy_drones_destroyed;
    unsigned int friendly_drones_destroyed;
    unsigned int healing_administred;
    unsigned int damage_dealt;
    unsigned int damage_received;
    unsigned int resources_scavenged;
    unsigned int resources_mined;
    unsigned int drones_recycled;
    unsigned int drones_recycled_resources;
    unsigned int javelins_fired;

} stats_t;

static stats_t stats;

void init_stats( )
{
   bzero( (void *)&stats, sizeof( stats ) );
}

unsigned int get_drones_assimilated( )
{
    return stats.drones_assimilated;
}

void add_drones_assimilated( unsigned int value )
{
    stats.drones_assimilated += value;
}

unsigned int get_enemy_drones_destroyed( )
{
    return stats.enemy_drones_destroyed;
}

void add_enemy_drones_destroyed( unsigned int value )
{
    stats.enemy_drones_destroyed += value;
}

unsigned int get_friendly_drones_destroyed( )
{
    return stats.friendly_drones_destroyed;
}

void add_friendly_drones_destroyed( unsigned int value )
{
    stats.friendly_drones_destroyed += value;
}

unsigned int get_healing_administred( )
{
    return stats.healing_administred;
}

void add_healing_administred( unsigned int value )
{
    stats.healing_administred += value;
}

unsigned int get_damage_dealt( )
{
    return stats.damage_dealt;
}

void add_damage_dealt( unsigned int value )
{
    stats.damage_dealt += value;
}

unsigned int get_damage_received( )
{
    return stats.damage_received;
}

void add_damage_received( unsigned int value )
{
    stats.damage_received += value;
}

unsigned int get_resources_scavenged( )
{
    return stats.resources_scavenged;
}

void add_resources_scavenged( unsigned int value )
{
    stats.resources_scavenged += value;
}

unsigned int get_resources_mined( )
{
    return stats.resources_mined;
}

void add_resources_mined( unsigned int value )
{
    stats.resources_mined += value;
}

unsigned int get_drones_recycled( )
{
    return stats.drones_recycled;
}

void add_drones_recycled( unsigned int value )
{
    stats.drones_recycled += value;
}

unsigned int get_drones_recycled_resources( )
{
    return stats.drones_recycled_resources;
}

void add_drones_recycled_resources( unsigned int value )
{
    stats.drones_recycled_resources += value;
}

unsigned int get_javelins_fired( )
{
    return stats.javelins_fired;
}

void add_javelins_fired( unsigned int value )
{
    stats.javelins_fired += value;
}


