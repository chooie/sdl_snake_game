#include "common.h"

typedef enum {
    DIRECTION_NONE,  // Represents no movement
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

// TODO move this somewhere
#define MAX_INPUTS 10
Direction input_queue[MAX_INPUTS];
int32 head = 0; // Points to the current input to be processed
int32 tail = 0; // Points to the next free spot for adding input

void add_input(Direction dir)
{
    int32 next_tail = (tail + 1) % MAX_INPUTS;
    if (next_tail != head) // Only add if there's space in the queue
    {
        input_queue[tail] = dir;
        tail = next_tail;
    }
}

Direction get_next_input()
{
    if (head == tail)
    {
        // No inputs available, return current direction
        return DIRECTION_NONE;
    }
    Direction dir = input_queue[head];
    head = (head + 1) % MAX_INPUTS;
    return dir;
}

void simulate(State* state, real64 simulation_time_elapsed, real32 dt_s)
{
    state->time_until_grid_jump__seconds -= dt_s;

    if (state->time_until_grid_jump__seconds <= 0)
    {
        Direction proposed_direction = get_next_input();

        if (!state->direction_locked) {
            switch (proposed_direction)
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