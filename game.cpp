#include "common.h"

typedef enum {
    DIRECTION_NORTH,
    DIRECTION_EAST,
    DIRECTION_SOUTH,
    DIRECTION_WEST
} Direction;

struct State {
    int32 pos_x;
    int32 pos_y;

    Direction direction;
    bool32 direction_locked;
    // real32 velocity_x;
    // real32 velocity_y;
    real32 time_until_grid_jump__seconds;
    real32 set_time_until_grid_jump__seconds;

    // Overload * operator for scalar multiplication
    State operator*(real32 scalar) const
    {
        State result;
        // result.pos_x = pos_x * scalar;
        // result.pos_y = pos_y * scalar;
        // result.velocity_x = velocity_x * scalar;
        // result.velocity_y = velocity_y * scalar;
        return result;
    }

    // Overload + operator for adding two States
    State operator+(const State& other) const
    {
        State result;
        // NOTE: I've commented this out because I don't think we need linear interpolation for new grid-based approach?
        // result.pos_x = pos_x + other.pos_x;
        // result.pos_y = pos_y + other.pos_y;
        // result.velocity_x = velocity_x + other.velocity_x;
        // result.velocity_y = velocity_y + other.velocity_y;
        return result;
    }
};

void simulate(State* state, Input* input, real64 simulation_time_elapsed, real32 dt_s)
{
    state->time_until_grid_jump__seconds -= dt_s;

    // TODO: should only be able to read one button at a time?
    if (is_down(BUTTON_W) && !state->direction_locked)
    {
        if (state->direction != DIRECTION_SOUTH)
        {
            state->direction = DIRECTION_NORTH;
            state->direction_locked = 1;
        }
    }
    
    if (is_down(BUTTON_A) && !state->direction_locked)
    {
        if (state->direction != DIRECTION_EAST)
        {
            state->direction = DIRECTION_WEST;
            state->direction_locked = 1;
        }
    }
    
    if (is_down(BUTTON_S) && !state->direction_locked)
    {
        if (state->direction != DIRECTION_NORTH)
        {
            state->direction = DIRECTION_SOUTH;
            state->direction_locked = 1;
        }
    }
    
    if (is_down(BUTTON_D) && !state->direction_locked)
    {
        if (state->direction != DIRECTION_WEST)
        {
            state->direction = DIRECTION_EAST;
            state->direction_locked = 1;
        }
    }

    if (state->direction_locked) {
        if (released(BUTTON_W)) {
            if (state->direction == DIRECTION_NORTH)
            {
                state->direction_locked = 0;
            }
        } else if (released(BUTTON_A))
        {
            if (state->direction == DIRECTION_WEST)
            {
                state->direction_locked = 0;
            }
        } else if (released(BUTTON_S))
        {
            if (state->direction == DIRECTION_SOUTH)
            {
                state->direction_locked = 0;
            }
        } else if (released(BUTTON_D))
        {
            if (state->direction == DIRECTION_EAST)
            {
                state->direction_locked = 0;
            }
        }
    }

    if (state->time_until_grid_jump__seconds <= 0)
    {
        switch (state->direction)
        {
            case DIRECTION_NORTH:
            {
                state->pos_y++;
            } break;
            case DIRECTION_EAST:
            {
                state->pos_x++;
            } break;
            case DIRECTION_SOUTH:
            {
                state->pos_y--;
            } break;
            case DIRECTION_WEST:
            {
                state->pos_x--;
            } break;
        }

        state->time_until_grid_jump__seconds = state->set_time_until_grid_jump__seconds;
        // printf("x: %d, y: %d\n", state->pos_x, state->pos_y);
    }

}