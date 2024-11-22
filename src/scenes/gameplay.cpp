#include <SDL2/SDL.h>

#include "../audio.h"
#include "../common.h"

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

struct Gameplay__Texts
{
    Drawn_Text_Static score_drawn_text_static;
    Drawn_Text_Int32 score_drawn_text_dynamic;
    Drawn_Text_Static game_over_drawn_text_static;
    Drawn_Text_Static restart_drawn_text_static;
    Drawn_Text_Static game_paused_drawn_text_static;
};

struct Gameplay__State
{
    bool32 is_starting;
    bool32 game_over;
    bool32 is_paused;

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

    Gameplay__Texts* gameplay_texts;

    // Overload * operator for scalar multiplication
    Gameplay__State operator*(real32 scalar) const
    {
        Gameplay__State result;
        // result.pos_x = pos_x * scalar;
        // result.pos_y = pos_y * scalar;
        // result.velocity_x = velocity_x * scalar;
        // result.velocity_y = velocity_y * scalar;
        return result;
    }

    // Overload + operator for adding two States
    Gameplay__State operator+(const Gameplay__State& other) const
    {
        Gameplay__State result;
        // NOTE: I've commented this out because I don't think we need linear interpolation for new grid-based approach?
        // result.pos_x = pos_x + other.pos_x;
        // result.pos_y = pos_y + other.pos_y;
        // result.velocity_x = velocity_x + other.velocity_x;
        // result.velocity_y = velocity_y + other.velocity_y;
        return result;
    }
};

// TODO move this somewhere
// TODO should this be part of Gameplay__State?
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

void gameplay__reset_state(Scene* scene)
{
    Gameplay__State* state = (Gameplay__State*)scene->state;

    head = 0;
    tail = 0;

    state->is_starting = 1;
    state->is_paused = 1;
    state->game_over = 0;

    state->pos_x = X_GRIDS / 2;
    state->pos_y = Y_GRIDS / 4;
    state->current_direction = DIRECTION_NORTH;
    state->next_snake_part_index = 0;

    state->set_time_until_grid_jump__seconds = .1f;
    state->time_until_grid_jump__seconds = state->set_time_until_grid_jump__seconds;

    state->blip_pos_x = X_GRIDS / 2;
    state->blip_pos_y = Y_GRIDS / 2;
}

Gameplay__Texts gameplay__setup_text()
{
    SDL_Color white_text_color = {255, 255, 255, 255};  // White color
    real32 font_size = 16.0f;

    Drawn_Text_Static score_drawn_text_static = {};
    {
        score_drawn_text_static.text_string = "SCORE";
    }
    score_drawn_text_static.font_size = font_size;
    score_drawn_text_static.color = white_text_color;

    Drawn_Text_Static game_paused_drawn_text_static = {};
    {
        game_paused_drawn_text_static.text_string = "GAME PAUSED";
    }
    game_paused_drawn_text_static.font_size = font_size * 2.f;
    game_paused_drawn_text_static.color = white_text_color;
    game_paused_drawn_text_static.text_rect.x = -LOGICAL_WIDTH; // Draw off-screen initially;


    Drawn_Text_Static game_over_drawn_text_static = {};
    {
        game_over_drawn_text_static.text_string = "GAME OVER";
    }
    game_over_drawn_text_static.font_size = font_size * 2.f;
    game_over_drawn_text_static.color = white_text_color;
    game_over_drawn_text_static.text_rect.x = -LOGICAL_WIDTH; // Draw off-screen initially;


    Drawn_Text_Static restart_drawn_text_static = {};
    {
        restart_drawn_text_static.text_string = "Press <Enter> to restart.";
    }
    restart_drawn_text_static.font_size = font_size;
    restart_drawn_text_static.color = white_text_color;
    restart_drawn_text_static.text_rect.x = -LOGICAL_WIDTH; // Draw off-screen initially;


    char dynamic_score_text[DYNAMIC_SCORE_LENGTH]; // Make sure the buffer is large enough
    Drawn_Text_Int32 score_drawn_text_dynamic = {};
    score_drawn_text_dynamic.original_value = -1;
    score_drawn_text_dynamic.text_string = dynamic_score_text;
    score_drawn_text_dynamic.font_size = font_size;
    score_drawn_text_dynamic.color = white_text_color;


    Gameplay__Texts gameplay_texts = {};
    gameplay_texts.score_drawn_text_static = score_drawn_text_static;
    gameplay_texts.score_drawn_text_dynamic = score_drawn_text_dynamic;
    gameplay_texts.game_over_drawn_text_static = game_over_drawn_text_static;
    gameplay_texts.restart_drawn_text_static = restart_drawn_text_static;
    gameplay_texts.game_paused_drawn_text_static = game_paused_drawn_text_static;

    return gameplay_texts;
}

void gameplay__handle_input(Scene* scene, Input* input)
{
    Gameplay__State* state = (Gameplay__State*)scene->state;

    if (pressed(BUTTON_ESCAPE))
    {
        global_current_scene = &global_start_screen_scene;
        stop_music();
    }

    if (pressed(BUTTON_SPACE) && !state->game_over)
    {
        state->is_paused = !state->is_paused;
    }

    if (!state->is_paused)
    {
        if (pressed(BUTTON_W) || pressed(BUTTON_UP))
        {
            add_input(DIRECTION_NORTH);
        }

        if (pressed(BUTTON_A) || pressed(BUTTON_LEFT))
        {
            add_input(DIRECTION_WEST);
        }

        if (pressed(BUTTON_S) || pressed(BUTTON_DOWN))
        {
            add_input(DIRECTION_SOUTH);
        }

        if (pressed(BUTTON_D) || pressed(BUTTON_RIGHT))
        {
            add_input(DIRECTION_EAST);
        }
    }

    if (state->game_over && pressed(BUTTON_ENTER))
    {
        gameplay__reset_state(scene);
        state->is_paused = 0;
    }
}

//=======================================================
// UPDATE
//=======================================================

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

void gameplay__update(struct Scene* scene, real64 simulation_time_elapsed, real32 dt_s)
{
    Gameplay__State* state = (Gameplay__State*)scene->state;

    if (state->is_starting)
    {
        state->is_starting = 0;
        play_music(&global_audio_context);
        set_music_volume(10.f);
    }

    if (state->game_over || state->is_paused)
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
                play_sound_effect(global_audio_context.effect_beep_2);

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
                play_sound_effect(global_audio_context.effect_boom);
            }
        }

        state->direction_locked = 0;
        state->time_until_grid_jump__seconds = state->set_time_until_grid_jump__seconds;
        // printf("x: %d, y: %d, %d, %d\n", state->pos_x, state->pos_y, state->current_direction,
        // state->proposed_direction);
    }
}

//=======================================================
// RENDER
//=======================================================

SDL_Texture* grid_texture = NULL;
int grid_texture_initialized = 0;

// Function to draw the grid onto a texture for caching
void create_grid_texture(SDL_Renderer* renderer)
{
    uint32 border_thickness = 1;                // Thickness of the white border
    SDL_Color grey_color = {40, 40, 40, 255};   // Dark grey color
    SDL_Color white_color = {60, 60, 60, 255};  // Lighter color for borders

    // Calculate the grid texture size
    int grid_width = X_GRIDS * GRID_BLOCK_SIZE;
    int grid_height = Y_GRIDS * GRID_BLOCK_SIZE;

    // Create the texture
    grid_texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, grid_width, grid_height);
    if (!grid_texture)
    {
        fprintf(stderr, "Failed to create grid texture: %s\n", SDL_GetError());
        return;
    }

    // Set the grid texture as the render target
    SDL_SetRenderTarget(renderer, grid_texture);

    // Draw the grid
    for (uint32 row_index = 0; row_index < Y_GRIDS; row_index++)
    {
        for (uint32 column_index = 0; column_index < X_GRIDS; column_index++)
        {
            SDL_Rect grid_block;
            grid_block.x = (int32)(column_index * GRID_BLOCK_SIZE);
            grid_block.y = (int32)(row_index * GRID_BLOCK_SIZE);
            grid_block.w = (int32)(GRID_BLOCK_SIZE);
            grid_block.h = (int32)(GRID_BLOCK_SIZE);

            // Draw the white border rectangle
            SDL_SetRenderDrawColor(renderer, white_color.r, white_color.g, white_color.b, white_color.a);
            SDL_RenderFillRect(renderer, &grid_block);

            // Shrink the grey rectangle by the border thickness to draw it inside the border
            SDL_Rect inner_block;
            inner_block.x = (int32)(grid_block.x + border_thickness);
            inner_block.y = (int32)(grid_block.y + border_thickness);
            inner_block.w = (int32)(grid_block.w - (2 * border_thickness));
            inner_block.h = (int32)(grid_block.h - (2 * border_thickness));

            // Draw the dark grey fill within the border
            SDL_SetRenderDrawColor(renderer, grey_color.r, grey_color.g, grey_color.b, grey_color.a);
            SDL_RenderFillRect(renderer, &inner_block);
        }
    }

    // Reset the rendering target to the default (screen)
    SDL_SetRenderTarget(renderer, NULL);

    grid_texture_initialized = 1;
}

// Function to render the grid by reusing the cached texture
void render_grid(SDL_Renderer* renderer)
{
    // Create the grid texture if it hasn't been created yet
    if (!grid_texture_initialized)
    {
        create_grid_texture(renderer);
    }

    // Render the cached grid texture to the screen
    SDL_RenderCopy(renderer, grid_texture, NULL, NULL);
}

void gameplay__render(Scene* scene)
{
    Gameplay__State* state = (Gameplay__State*)scene->state;
    Gameplay__Texts* gameplay_texts = state->gameplay_texts;

    draw_canvas();

    render_grid(global_renderer);

    {  // Draw Blip
        Screen_Space_Position square_screen_pos =
            map_world_space_position_to_screen_space_position(state->blip_pos_x, state->blip_pos_y);

        real32 size = GRID_BLOCK_SIZE * 0.5f;

        SDL_Rect square = {};
        square.x = (int32)(square_screen_pos.x + ((real32)GRID_BLOCK_SIZE / 2) - (size / 2));
        square.y = (int32)(square_screen_pos.y + ((real32)GRID_BLOCK_SIZE / 2) - (size / 2));
        square.w = (int32)size;
        square.h = (int32)size;

        SDL_Color color = {52, 152, 219, 255};
        draw_rect(square, color);
    }

    {  // Draw Player
        Screen_Space_Position square_screen_pos =
            map_world_space_position_to_screen_space_position(state->pos_x, state->pos_y);

        SDL_Rect square = {};
        square.x = (int32)(square_screen_pos.x);
        square.y = (int32)(square_screen_pos.y);
        square.w = (int32)GRID_BLOCK_SIZE;
        square.h = (int32)GRID_BLOCK_SIZE;

        SDL_Color red = {171, 70, 66, 255};
        draw_rect(square, red);

        for (uint32 i = 0; i < state->next_snake_part_index; i++)
        {
            Snake_Part* snake_part = &state->snake_parts[i];
            Screen_Space_Position screen_pos =
                map_world_space_position_to_screen_space_position(snake_part->pos_x, snake_part->pos_y);

            SDL_Rect square = {};
            square.x = (int32)(screen_pos.x);
            square.y = (int32)(screen_pos.y);
            square.w = (int32)GRID_BLOCK_SIZE;
            square.h = (int32)GRID_BLOCK_SIZE;

            SDL_Color darkened_red = {154, 63, 59, 255};
            draw_rect(square, darkened_red);
        }
    }

    {  // Render score
        int32 OFFSET = 40;
        gameplay_texts->score_drawn_text_static.text_rect.x =
            LOGICAL_WIDTH - gameplay_texts->score_drawn_text_static.text_rect.w - OFFSET;
        gameplay_texts->score_drawn_text_static.text_rect.y = 0;
        draw_text_static(&gameplay_texts->score_drawn_text_static);

        // ==========================

        if (state->next_snake_part_index != gameplay_texts->score_drawn_text_dynamic.original_value)
        {
            snprintf(gameplay_texts->score_drawn_text_dynamic.text_string,
                     DYNAMIC_SCORE_LENGTH,
                     "%d",
                     state->next_snake_part_index);
        }

        gameplay_texts->score_drawn_text_dynamic.text_rect.x = gameplay_texts->score_drawn_text_static.text_rect.x +
                                                                5 +
                                                                gameplay_texts->score_drawn_text_static.text_rect.w;
        gameplay_texts->score_drawn_text_dynamic.text_rect.y = 0;
        draw_text_int32(&gameplay_texts->score_drawn_text_dynamic, state->next_snake_part_index);
    }

    {  // Render Game Over
        if (state->game_over)
        {
            draw_text_static(&gameplay_texts->game_over_drawn_text_static);
            gameplay_texts->game_over_drawn_text_static.text_rect.x = LOGICAL_WIDTH / 2;
            gameplay_texts->game_over_drawn_text_static.text_rect.y = LOGICAL_HEIGHT / 2;

            gameplay_texts->game_over_drawn_text_static.text_rect.x -=
                gameplay_texts->game_over_drawn_text_static.text_rect.w / 2;
            gameplay_texts->game_over_drawn_text_static.text_rect.y -=
                gameplay_texts->game_over_drawn_text_static.text_rect.h / 2;

            draw_text_static(&gameplay_texts->restart_drawn_text_static);
            gameplay_texts->restart_drawn_text_static.text_rect.x = LOGICAL_WIDTH / 2;
            gameplay_texts->restart_drawn_text_static.text_rect.y = LOGICAL_HEIGHT / 2;
            gameplay_texts->restart_drawn_text_static.text_rect.x -=
                gameplay_texts->restart_drawn_text_static.text_rect.w / 2;
            gameplay_texts->restart_drawn_text_static.text_rect.y -=
                gameplay_texts->restart_drawn_text_static.text_rect.h / 2;

            // Place text under other text
            gameplay_texts->restart_drawn_text_static.text_rect.y +=
                gameplay_texts->game_over_drawn_text_static.text_rect.h;
        }
    }

    {  // Render Game Paused
        if (state->is_paused)
        {
            draw_text_static(&gameplay_texts->game_paused_drawn_text_static);
            gameplay_texts->game_paused_drawn_text_static.text_rect.x = LOGICAL_WIDTH / 2;
            gameplay_texts->game_paused_drawn_text_static.text_rect.y = LOGICAL_HEIGHT / 2;

            gameplay_texts->game_paused_drawn_text_static.text_rect.x -=
                gameplay_texts->game_paused_drawn_text_static.text_rect.w / 2;
            gameplay_texts->game_paused_drawn_text_static.text_rect.y -=
                gameplay_texts->game_paused_drawn_text_static.text_rect.h / 2;
        }
    }
}