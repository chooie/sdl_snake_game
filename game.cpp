#include "audio.h"
#include "common.h"

// Define parameters for the LCG
uint32 seed = 12345;  // Initial seed value
#define LCG_A 1664525u
#define LCG_C 1013904223u

// Function to generate a pseudorandom number using LCG
uint32 custom_rand()
{
    seed = LCG_A * seed + LCG_C;
    return seed;
}

// Function to generate a number between 0 and max - 1 (inclusive)
uint32 custom_rand_range(uint32 max)
{
    return custom_rand() % (max);
}

typedef enum
{
    DIRECTION_NONE,  // Represents no movement
    DIRECTION_NORTH,
    DIRECTION_EAST,
    DIRECTION_SOUTH,
    DIRECTION_WEST
} Direction;

struct Snake_Part
{
    int32 pos_x;
    int32 pos_y;
    Direction direction;
};

#define MAX_TAIL_LENGTH 1000

struct Gameplay_State
{
    bool32 game_over;

    int32 pos_x;
    int32 pos_y;

    Snake_Part snake_parts[MAX_TAIL_LENGTH];
    uint32 next_snake_part_index;

    Direction current_direction;
    Direction proposed_direction;
    bool32 direction_locked;

    real32 time_until_grid_jump__seconds;
    real32 set_time_until_grid_jump__seconds;

    int32 blip_pos_x;
    int32 blip_pos_y;

    // Overload * operator for scalar multiplication
    Gameplay_State operator*(real32 scalar) const
    {
        Gameplay_State result;
        // result.pos_x = pos_x * scalar;
        // result.pos_y = pos_y * scalar;
        // result.velocity_x = velocity_x * scalar;
        // result.velocity_y = velocity_y * scalar;
        return result;
    }

    // Overload + operator for adding two States
    Gameplay_State operator+(const Gameplay_State& other) const
    {
        Gameplay_State result;
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
int32 head = 0;  // Points to the current input to be processed
int32 tail = 0;  // Points to the next free spot for adding input

void add_input(Direction dir)
{
    int32 next_tail = (tail + 1) % MAX_INPUTS;
    if (next_tail != head)  // Only add if there's space in the queue
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

void simulate(Gameplay_State* state, Audio_Context* audio_context, real64 simulation_time_elapsed, real32 dt_s)
{
    if (state->game_over)
    {
        return;
    }

    state->time_until_grid_jump__seconds -= dt_s;

    if (state->time_until_grid_jump__seconds <= 0)
    {
        Direction proposed_direction = get_next_input();

        if (!state->direction_locked)
        {
            switch (proposed_direction)
            {
                case DIRECTION_NORTH:
                {
                    if (state->current_direction != DIRECTION_SOUTH)
                    {
                        state->current_direction = DIRECTION_NORTH;
                    }
                }
                break;
                case DIRECTION_EAST:
                {
                    if (state->current_direction != DIRECTION_WEST)
                    {
                        state->current_direction = DIRECTION_EAST;
                    }
                }
                break;
                case DIRECTION_SOUTH:
                {
                    if (state->current_direction != DIRECTION_NORTH)
                    {
                        state->current_direction = DIRECTION_SOUTH;
                    }
                }
                break;
                case DIRECTION_WEST:
                {
                    if (state->current_direction != DIRECTION_EAST)
                    {
                        state->current_direction = DIRECTION_WEST;
                    }
                }
                break;

                    state->direction_locked = 1;
            }
        }

        {  // Blip collision
            if (state->pos_x == state->blip_pos_x && state->pos_y == state->blip_pos_y)
            {
                play_sound_effect(audio_context->effect_beep);

                {  // Grow snake part
                    Snake_Part new_snake_part = {};
                    Snake_Part* last_snake_part = &state->snake_parts[state->next_snake_part_index];
                    new_snake_part.pos_x = last_snake_part->pos_x;
                    new_snake_part.pos_y = last_snake_part->pos_y;
                    new_snake_part.direction = last_snake_part->direction;
                    state->next_snake_part_index++;

                    if (!(state->next_snake_part_index < MAX_TAIL_LENGTH))
                    {
                        SDL_SetError("Snake tail must never get this long! %d", state->next_snake_part_index);
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
                        SDL_assert_release(state->next_snake_part_index < MAX_TAIL_LENGTH);
                    }
                }

                {  // Randomly spawn blip somewhere else
                    uint32 random_x = custom_rand_range(X_GRIDS);
                    state->blip_pos_x = random_x;

                    uint32 random_y = custom_rand_range(Y_GRIDS);
                    state->blip_pos_y = random_y;
                }

                state->set_time_until_grid_jump__seconds -= 0.0005;
            }
        }

        for (int32 i = state->next_snake_part_index - 1; i >= 0; i--)
        {
            Snake_Part* current_snake_part = &state->snake_parts[i];

            if (i == 0)
            {
                current_snake_part->pos_x = state->pos_x;
                current_snake_part->pos_y = state->pos_y;
                current_snake_part->direction = state->current_direction;
            }
            else
            {
                Snake_Part* previous_snake_part = &state->snake_parts[i - 1];
                current_snake_part->pos_x = previous_snake_part->pos_x;
                current_snake_part->pos_y = previous_snake_part->pos_y;
                current_snake_part->direction = previous_snake_part->direction;
            }
        }

        switch (state->current_direction)
        {
            case DIRECTION_NORTH:
            {
                if (state->current_direction != DIRECTION_SOUTH)
                {
                    state->pos_y++;
                }
            }
            break;
            case DIRECTION_EAST:
            {
                if (state->current_direction != DIRECTION_WEST)
                {
                    state->pos_x++;
                }
            }
            break;
            case DIRECTION_SOUTH:
            {
                if (state->current_direction != DIRECTION_NORTH)
                {
                    state->pos_y--;
                }
            }
            break;
            case DIRECTION_WEST:
            {
                if (state->current_direction != DIRECTION_EAST)
                {
                    state->pos_x--;
                }
            }
            break;
        }

        {  // End Game if player crashes
            for (int32 i = 0; i < state->next_snake_part_index; i++)
            {
                Snake_Part* current_snake_part = &state->snake_parts[i];
                if (state->pos_x == current_snake_part->pos_x && state->pos_y == current_snake_part->pos_y)
                {
                    state->game_over = 1;
                    break;
                }
            }

            if (state->pos_x < 0 || state->pos_x >= X_GRIDS || state->pos_y < 0 || state->pos_y >= Y_GRIDS)
            {
                state->game_over = 1;
            }

            if (state->game_over)
            {
                play_sound_effect(audio_context->effect_boom);
            }
        }

        state->direction_locked = 0;
        state->time_until_grid_jump__seconds = state->set_time_until_grid_jump__seconds;
        // printf("x: %d, y: %d, %d, %d\n", state->pos_x, state->pos_y, state->current_direction,
        // state->proposed_direction);
    }
}