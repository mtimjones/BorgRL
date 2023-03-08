// behaviors.c
//
// This file implements the drone behaviors (behavior per class of drone).

#include "headers.h"
#include "ces.h"
#include "logapi.h"
#include "stats.h"

void extract_behavior( int entity, int target_entity )
{
    // FSM for the extracting drone (scav/mine).  Started when a object is targeted.

    switch( get_entity_behavior_state( entity ) )
    {
        case Dormant:
            // Check if the target is a wreck.  If not, stay dormant and disable behavior.
            if ( is_wreck( target_entity ) || ( is_planet( target_entity ) ) )
            {
                set_cached_entity_target( entity, target_entity );
                set_target_entity( entity, TYPE_ENTITY_TARGET, target_entity );
                set_entity_behavior_state( entity, TravelingToResources );
                set_entity_state_str( entity, "Traveling " );
            }
            else if ( is_academy( target_entity ) )
            {
                set_cached_entity_target( entity, target_entity );
                set_target_entity( entity, TYPE_ENTITY_TARGET, target_entity );
                set_entity_behavior_state( entity, TravelingToAcademy );
                set_entity_state_str( entity, "Traveling " );
            }
            break;

        case TravelingToResources:
            if ( entity_distance( entity, target_entity ) == 1 )
            {
                set_entity_behavior_state( entity, ScavengingResources );
                clear_entity_target( entity );
                set_entity_state_str( entity, get_edrone_action_str( entity ) );
            }
            break;

        case ScavengingResources:
            if ( get_entity_resources( target_entity ) > 0 )
            {
                // Target has resources, commence/continue mining.
                if ( get_edrone_state( entity ) == 0 )
                {
                    set_edrone_state( entity, get_edrone_speed( entity ) );
                    set_entity_resources( target_entity, get_entity_resources( target_entity ) - 1 );
                    set_entity_resources( entity, get_entity_resources( entity ) + 1 );

                    if ( get_entity_resources( target_entity ) == 0 )
                    {
                        // Target is empty.
                        demote_wreck_entity( target_entity );
                        set_target_entity( entity, TYPE_ENTITY_DOCK, PLAYER_ID );
                        set_entity_behavior_state( entity, RedockingWithBorg );
                        set_entity_state_str( entity, "Docking   " );
                    }
                    else if ( get_entity_resources( entity ) >= get_edrone_max_resources( entity ) )
                    {
                        // Drone is full, return to the Borg.
                        set_target_entity( entity, TYPE_ENTITY_TARGET, PLAYER_ID );
                        set_entity_behavior_state( entity, ReturningToBorg );
                        set_entity_state_str( entity, "Returning " );
                    }
                }
                else
                {
                    // Decrement...
                    set_edrone_state( entity, get_edrone_state( entity ) - 1 );
                }
            }
            else
            {
                // Target is now empty.
                demote_wreck_entity( entity );
                set_target_entity( entity, TYPE_ENTITY_DOCK, PLAYER_ID );
                set_entity_behavior_state( entity, RedockingWithBorg );
                set_entity_state_str( entity, "Docking   " );
            }
            break;

        case ReturningToBorg:
            if ( entity_distance( entity, PLAYER_ID ) == 1 )
            {
                if ( is_entity_scavenger( entity ) ) add_resources_scavenged( get_entity_resources( entity ) );
                else add_resources_mined( get_entity_resources( entity ) );

                int resources = get_entity_resources( PLAYER_ID ) + get_entity_resources( entity );
                set_entity_resources( PLAYER_ID, resources );

                add_xp( entity, get_entity_resources( entity ) );
                add_xp( PLAYER_ID, 1 );

                set_entity_resources( entity, 0 );
                set_target_entity( entity, TYPE_ENTITY_TARGET, get_cached_entity_target( entity ) );
                set_entity_behavior_state( entity, TravelingToResources );
                set_entity_state_str( entity, "Returning " );
            }
            break;

        case RedockingWithBorg:
            if ( entity_distance( entity, PLAYER_ID ) == 1 )
            {
                set_entity_behavior_state( entity, Dormant );
                stop_entity_behavior( entity );

                if ( is_entity_scavenger( entity ) ) add_resources_scavenged( get_entity_resources( entity ) );
                else add_resources_mined( get_entity_resources( entity ) );

                // Move resources from drone to Borg.
                int resources = get_entity_resources( PLAYER_ID ) + get_entity_resources( entity );
                set_entity_resources( PLAYER_ID, resources );
                add_xp( entity, get_entity_resources( entity ) );
                add_xp( PLAYER_ID, 1 );
                set_entity_resources( entity, 0 );
                set_entity_behavior_state( entity, Dormant );
                set_entity_state_str( entity, "Docked    " );
            }
            break;

        case TravelingToAcademy:
            if ( entity_distance( entity, target_entity ) == 1 )
            {
                set_target_entity( entity, TYPE_ENTITY_DOCK, PLAYER_ID );
                set_entity_behavior_state( entity, ReturningFromAcademy );
                set_entity_state_str( entity, "Redocking " );

                academy_edrone_upgrade( entity );
            }
            break;

        case ReturningFromAcademy:
            if ( entity_distance( entity, PLAYER_ID ) == 1 )
            {
                set_entity_behavior_state( entity, Dormant );
                stop_entity_behavior( entity );
                add_xp( PLAYER_ID, 1 );
                set_entity_state_str( entity, "Docked    " );
            }
            break;

        default:
            assert( 0 );
            break;

    }

    return;
}

void enemy_combat_behavior( int entity, int target_entity )
{
    // FSM for the combat drone.  Started when a object is targeted.
    // Note that target entity is passed in undefined, but will be set here once something is in range.

    switch( get_entity_behavior_state( entity ) )
    {
        case Dormant:
            target_entity = find_close_entity( entity );

            if ( target_entity != NO_ENTITY )
            {
                // Found a close entity, set it as the target.
                set_cached_entity_target( entity, target_entity );
                set_target_entity( entity, TYPE_ENTITY_TARGET, target_entity );
                set_entity_behavior_state( entity, MovingTowardsTarget );
            }
            break;

        case MovingTowardsTarget:
            if ( entity_distance( entity, target_entity ) == 1 )
            {
                set_entity_behavior_state( entity, AttackingTarget );
                clear_entity_target( entity );
            }
            else if ( entity_distance( entity, target_entity ) > MAX_RADAR_DISTANCE )
            {
                stop_entity_behavior( entity );
            }
            break;

        case AttackingTarget:
            if ( entity_distance( entity, target_entity ) == 1 )
            {
                attack( entity, target_entity );
            }
            else
            {
                set_target_entity( entity, TYPE_ENTITY_TARGET, target_entity );
                set_entity_behavior_state( entity, MovingTowardsTarget );
            }
            break;

        default:
            set_entity_behavior_state( entity, Dormant );
            stop_entity_behavior( entity );
            break;
    }

}

void friendly_combat_behavior( int entity, int target_entity )
{
    // FSM for the friendly combat drone.  Started when a object is targeted.

    switch( get_entity_behavior_state( entity ) )
    {
        case Dormant:
            set_entity_state_str( entity, "Undocked  " );
            if ( target_entity != NO_ENTITY )
            {
                set_entity_state_str( entity, "Traveling " );
                if ( is_academy( target_entity ) )
                {
                    set_cached_entity_target( entity, target_entity );
                    set_target_entity( entity, TYPE_ENTITY_TARGET, target_entity );
                    set_entity_behavior_state( entity, TravelingToAcademy );
                }
                else
                {
                    set_cached_entity_target( entity, target_entity );
                    set_target_entity( entity, TYPE_ENTITY_TARGET, target_entity );
                    set_entity_behavior_state( entity, MovingTowardsTarget );
                }
            }
            break;

        case MovingTowardsTarget:
            if ( entity_distance( entity, target_entity ) == 1 )
            {
                set_entity_behavior_state( entity, AttackingTarget );
                clear_entity_target( entity );
            }
            else if ( entity_distance( entity, target_entity ) > MAX_RADAR_DISTANCE )
            {
                clear_entity_target( entity );
                set_entity_behavior_state( entity, Dormant );
            }
            break;

        case AttackingTarget:
            if ( entity_distance( entity, target_entity ) == 1 )
            {
                set_entity_state_str( entity, "Attacking " );
                attack( entity, target_entity );
            }
            else
            {
                set_entity_state_str( entity, "Traveling " );
                set_target_entity( entity, TYPE_ENTITY_TARGET, target_entity );
                set_entity_behavior_state( entity, MovingTowardsTarget );
            }
            break;

        case TravelingToAcademy:
            if ( entity_distance( entity, target_entity ) == 1 )
            {
                set_target_entity( entity, TYPE_ENTITY_DOCK, PLAYER_ID );
                set_entity_behavior_state( entity, ReturningFromAcademy );
                set_entity_state_str( entity, "Redocking " );

                academy_cdrone_upgrade( entity );
            }
            break;

        case ReturningFromAcademy:
            if ( entity_distance( entity, PLAYER_ID ) == 1 )
            {
                set_entity_behavior_state( entity, Dormant );
                stop_entity_behavior( entity );
                add_xp( PLAYER_ID, 1 );
                set_entity_state_str( entity, "Docked    " );
            }
            break;

        default:
            set_entity_state_str( entity, "Unknown   " );
            set_entity_behavior_state( entity, Dormant );
            stop_entity_behavior( entity );
            break;
    }

}

void javelin_behavior( int entity, int target_entity )
{
    // FSM for the friendly javelin combat drone.  Started when a object is targeted.

    switch( get_entity_behavior_state( entity ) )
    {
        case Dormant:
            if ( target_entity != NO_ENTITY )
            {
                add_javelins_fired( 1 );
                set_entity_state_str( entity, "Traveling " );
                set_cached_entity_target( entity, target_entity );
                set_target_entity( entity, TYPE_ENTITY_TARGET, target_entity );
                set_entity_behavior_state( entity, MovingTowardsTarget );
            }
            break;

        case MovingTowardsTarget:
            if ( entity_distance( entity, target_entity ) == 1 )
            {
                javelin_explosion( entity );
            }
            break;

        default:
            assert(0);
            break;

    }

}

void boss_behavior( int entity, int target_entity )
{
    static int drone_count = BOSS_DRONE_COUNT;
    static int drone_wait  = BOSS_DRONE_WAIT_TIME;

    (void) target_entity;

    // FSM for the final boss.

    switch( get_entity_behavior_state( entity ) )
    {
        case Dormant:
            if ( entity_distance( entity, PLAYER_ID ) < 20 )
            {
                add_message( "Boss is awake, prepare for drone wave attack." );
                set_entity_behavior_state( entity, Awake );
            }
            break;

        case Awake:
            if ( drone_wait-- == 0 )
            {
                int col, row, coff, roff;

                drone_wait = BOSS_DRONE_WAIT_TIME;

                // emit a drone

                col = get_entity_col( entity );
                row = get_entity_row( entity );

                do
                {
                    coff = getRand( 6 ) - 3;
                    roff = getRand( 4 ) - 2;
                } while ( !is_cell_empty( col+coff, row+roff ) );

                int level = get_max_level( );
                int index = getRand( 5 ) - 1;

                create_cedrone_entity( get_cdrone_type( get_level( ), index ), col+coff, row+roff, 0,
                                        get_cd_hp( level, index ), get_cd_move_speed( level, index ),
                                        get_cd_attack( level, index ), get_cd_attack_speed( level, index ),
                                        get_cd_armor( level, index ), get_cd_hp( level, index ) );

                if ( drone_count-- == 0 )
                {
                    add_message( "Boss is empty, Finish him." );
                    set_entity_behavior_state( entity, Done );
                }
            }
            break;

        case Done:
            // Now, just wait to die...
            break;

        default:
            assert(0);
            break;
    }

}

