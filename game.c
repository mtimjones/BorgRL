#include "headers.h"

static int GameRunning = 0;
static int Paused = 0;
static death_type_t How = None;

void start_game( void )
{
    GameRunning = 1;
}

void end_game( void )
{
    GameRunning = 0;
}

bool get_game_state( void )
{
    return GameRunning;
}

void pause_toggle( void )
{
    Paused = !Paused;
}

bool get_pause_state( void )
{
    return Paused;
}

void set_death_type( death_type_t how )
{
    How = how;
}

death_type_t get_death_type( void )
{
    return How;
}
