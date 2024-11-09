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

    Direction current_direction;
    Direction proposed_direction;
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

    // TODO: these controls don't feel good
    // Maybe we need to push the inputs to a queue and then process them each tick so we don't miss any?

    if (is_down(BUTTON_W))
    {
        state->proposed_direction = DIRECTION_NORTH;
    }
    
    if (is_down(BUTTON_A))
    {
        state->proposed_direction = DIRECTION_WEST;
    }

    if (is_down(BUTTON_S))
    {
        state->proposed_direction = DIRECTION_SOUTH;
    }

    if (is_down(BUTTON_D))
    {
        state->proposed_direction = DIRECTION_EAST;
    }
    
    if (state->time_until_grid_jump__seconds <= 0)
    {
        if (!state->direction_locked) {

            switch (state->proposed_direction)
            {
                case DIRECTION_NORTH:
                {
                    if (state->current_direction != DIRECTION_SOUTH) {
                        state->current_direction = DIRECTION_NORTH;
                    }
                } break;
                case DIRECTION_EAST:
                {
                    if (state->current_direction != DIRECTION_WEST) {
                        state->current_direction = DIRECTION_EAST;
                    }
                } break;
                case DIRECTION_SOUTH:
                {
                    if (state->current_direction != DIRECTION_NORTH) {
                        state->current_direction = DIRECTION_SOUTH;
                    }
                } break;
                case DIRECTION_WEST:
                {
                    if (state->current_direction != DIRECTION_EAST) {
                        state->current_direction = DIRECTION_WEST;
                    }
                } break;

                state->direction_locked = 1;
            }
        }

        switch (state->current_direction)
        {
            case DIRECTION_NORTH:
            {
                if (state->current_direction != DIRECTION_SOUTH) {
                    state->pos_y++;
                }
            } break;
            case DIRECTION_EAST:
            {
                if (state->current_direction != DIRECTION_WEST) {
                    state->pos_x++;
                }
            } break;
            case DIRECTION_SOUTH:
            {
                if (state->current_direction != DIRECTION_NORTH) {
                    state->pos_y--;
                }
            } break;
            case DIRECTION_WEST:
            {
                if (state->current_direction != DIRECTION_EAST) {
                    state->pos_x--;
                }
            } break;
        }

        state->direction_locked = 0;
        state->time_until_grid_jump__seconds = state->set_time_until_grid_jump__seconds;
        // printf("x: %d, y: %d, %d, %d\n", state->pos_x, state->pos_y, state->current_direction, state->proposed_direction);
    }

}