#include "common.h"

real32 velocity_power = 20.0f;

typedef enum {
    DIRECTION_NORTH,
    DIRECTION_EAST,
    DIRECTION_SOUTH,
    DIRECTION_WEST
} Direction;

struct State {
    real32 pos_x;
    real32 pos_y;

    Direction direction;
    real32 velocity_x;
    real32 velocity_y;

    // Overload * operator for scalar multiplication
    State operator*(real32 scalar) const
    {
        State result;
        result.pos_x = pos_x * scalar;
        result.pos_y = pos_y * scalar;
        result.direction = direction;
        result.velocity_x = velocity_x * scalar;
        result.velocity_y = velocity_y * scalar;
        return result;
    }

    // Overload + operator for adding two States
    State operator+(const State& other) const
    {
        State result;
        result.pos_x = pos_x + other.pos_x;
        result.pos_y = pos_y + other.pos_y;
        result.direction = direction;
        result.velocity_x = velocity_x + other.velocity_x;
        result.velocity_y = velocity_y + other.velocity_y;
        return result;
    }
};

void simulate(State* state, Input* input, real64 simulation_time_elapsed, real32 dt_s)
{

    // TODO: should only be able to read one button at a time?
    if (is_down(BUTTON_W))
    {
        if (state->direction != DIRECTION_SOUTH)
        {
            state->direction = DIRECTION_NORTH;
        }
        // printf("Holding down W...\n");
    }

    if (is_down(BUTTON_A))
    {
        if (state->direction != DIRECTION_EAST)
        {
            state->direction = DIRECTION_WEST;
        }
        // printf("Holding down A...\n");
    }

    if (is_down(BUTTON_S))
    {
        if (state->direction != DIRECTION_NORTH)
        {
            state->direction = DIRECTION_SOUTH;
        }
        // printf("Holding down S...\n");
    }

    if (is_down(BUTTON_D))
    {
        if (state->direction != DIRECTION_WEST)
        {
            state->direction = DIRECTION_EAST;
        }
        // printf("Holding down D...\n");
    }

    switch (state->direction)
    {
        case DIRECTION_NORTH:
        {
            state->velocity_x = 0;
            state->velocity_y = velocity_power;
        } break;
        case DIRECTION_EAST:
        {
            state->velocity_x = velocity_power;
            state->velocity_y = 0;
        } break;
        case DIRECTION_SOUTH:
        {
            state->velocity_x = 0;
            state->velocity_y = -velocity_power;
        } break;
        case DIRECTION_WEST:
        {
            state->velocity_x = -velocity_power;
            state->velocity_y = 0;
        } break;
    }

    state->pos_x += state->velocity_x * dt_s;
    state->pos_y += state->velocity_y * dt_s;

/*
    // Slow the player down through friction
    acceleration_x -= state->velocity_x * friction;
    acceleration_y -= state->velocity_y * friction;

    state->velocity_x += acceleration_x * dt_s;
    state->velocity_y += acceleration_y * dt_s;

    // Kinematic Formula
    state->pos_x += state->velocity_x * dt_s + (0.5f * acceleration_x * dt_s * dt_s);
    state->pos_y += state->velocity_y * dt_s + (0.5f * acceleration_y * dt_s * dt_s);

*/

    // printf("x: %.2f, y: %.2f\n", state->pos_x, state->pos_y);
}