#include <SDL2/SDL.h>

// TODO: Move this somewhere
bool32 are_colors_equal(SDL_Color color1, SDL_Color color2)
{
    return (color1.r == color2.r &&
            color1.g == color2.g &&
            color1.b == color2.b &&
            color1.a == color2.a);
}

struct Menu_Texts
{
    Drawn_Text_Static snake_game_text_static;
    Drawn_Text_Static_2 start_game_text_static;
    Drawn_Text_Static_2 exit_game_text_static;
};

local_internal SDL_Color yellow = {255, 255, 0, 255};   // Yellow
local_internal SDL_Color white = {255, 255, 255, 255};  // White

typedef enum
{
    Start_Screen_Option__Start_Game,
    Start_Screen_Option__Exit_Game,
} Start_Screen__Option;

struct Start_Screen__State
{
    Menu_Texts* menu_texts;
    SDL_Color blink_color;
    Start_Screen__Option current_option;
};

void start_screen__reset_state(Scene* scene)
{
    Start_Screen__State* state = (Start_Screen__State*)scene->state;
    state->blink_color = white;
    state->current_option = Start_Screen_Option__Start_Game;
}

Menu_Texts start_screen__setup_text()
{
    real32 font_size = 16.0f;

    Drawn_Text_Static snake_game_text_static = {};
    {
        snake_game_text_static.text_string = "Snake Game";
    }
    snake_game_text_static.font_size = font_size * 4.f;
    snake_game_text_static.color = white;
    snake_game_text_static.text_rect.x = -LOGICAL_WIDTH;  // Draw off-screen initially;

    Drawn_Text_Static_2 start_game_text_static = {};
    {
        start_game_text_static.text_string = "Start";
    }
    start_game_text_static.font_size = font_size * 1.5f;
    start_game_text_static.color = white;
    start_game_text_static.text_rect.x = -LOGICAL_WIDTH;  // Draw off-screen initially;

    Drawn_Text_Static_2 exit_game_text_static = {};
    {
        exit_game_text_static.text_string = "Exit";
    }
    exit_game_text_static.font_size = font_size * 1.5f;
    exit_game_text_static.color = white;
    exit_game_text_static.text_rect.x = -LOGICAL_WIDTH;  // Draw off-screen initially;

    Menu_Texts menu_texts = {};
    menu_texts.snake_game_text_static = snake_game_text_static;
    menu_texts.start_game_text_static = start_game_text_static;
    menu_texts.exit_game_text_static = exit_game_text_static;

    return menu_texts;
}

void start_screen__handle_input(Scene* scene, Input* input)
{
    Start_Screen__State* state = (Start_Screen__State*)scene->state;

    if (pressed(BUTTON_ENTER) && state->current_option == Start_Screen_Option__Start_Game)
    {
        global_next_scene = &global_gameplay_scene;
    }

    if (pressed(BUTTON_ENTER) && state->current_option == Start_Screen_Option__Exit_Game)
    {
        global_running = 0;
    }

    if (pressed(BUTTON_D) || pressed(BUTTON_A))
    {
        play_sound_effect(global_audio_context.effect_beep);
    }

    if (pressed(BUTTON_D))
    {
        if (state->current_option == Start_Screen_Option__Start_Game)
        {
            state->current_option = Start_Screen_Option__Exit_Game;
        }
        else if (state->current_option == Start_Screen_Option__Exit_Game)
        {
            state->current_option = Start_Screen_Option__Start_Game;
        }
    }

    if (pressed(BUTTON_A))
    {
        if (state->current_option == Start_Screen_Option__Start_Game)
        {
            state->current_option = Start_Screen_Option__Exit_Game;
        }
        else if (state->current_option == Start_Screen_Option__Exit_Game)
        {
            state->current_option = Start_Screen_Option__Start_Game;
        }
    }
}

real32 TICK_EVERY__SECONDS = 0.15f;
real32 global_tick_time_remaining = TICK_EVERY__SECONDS;
int32 global_marker;
int32 global_next_marker;

void start_screen__update(struct Scene* scene,
                          real64 simulation_time_elapsed,
                          real32 dt_s)
{
    Start_Screen__State* state = (Start_Screen__State*)scene->state;

    SDL_Color yellow = {196, 160, 3, 255};   // Yellow
    SDL_Color white = {255, 255, 255, 255};  // White

    if (global_tick_time_remaining <= 0)
    {
        global_tick_time_remaining = TICK_EVERY__SECONDS;

        uint32 seconds = (uint32)simulation_time_elapsed;

        if (global_marker == 0)
        {
            state->blink_color = white;
            global_next_marker = 1;
        }
        else if (global_marker == 1)
        {
            state->blink_color = yellow;
            global_next_marker = 0;
        }
    }

    global_tick_time_remaining -= dt_s;

    Menu_Texts* menu_texts = state->menu_texts;

    if (state->current_option == Start_Screen_Option__Start_Game)
    {
        if (global_next_marker != global_marker)
        {
            menu_texts->start_game_text_static.should_update = 1;
            menu_texts->start_game_text_static.color = state->blink_color;
            global_marker = global_next_marker;
        }
    }
    else
    {
        if (!are_colors_equal(menu_texts->start_game_text_static.color, white))
        {
            menu_texts->start_game_text_static.should_update = 1;
            menu_texts->start_game_text_static.color = white;
        }
    }

    if (state->current_option == Start_Screen_Option__Exit_Game)
    {
        if (global_next_marker != global_marker)
        {
            menu_texts->exit_game_text_static.should_update = 1;
            menu_texts->exit_game_text_static.color = state->blink_color;
            global_marker = global_next_marker;
        }
    }
    else
    {
        if (!are_colors_equal(menu_texts->exit_game_text_static.color, white))
        {
            menu_texts->exit_game_text_static.should_update = 1;
            menu_texts->exit_game_text_static.color = white;
        }
    }
}

void start_screen__render(Scene* scene)
{
    Start_Screen__State* state = (Start_Screen__State*)scene->state;
    Menu_Texts* menu_texts = state->menu_texts;

    draw_canvas();

    {  // Render Snake Game
        draw_text_static(&menu_texts->snake_game_text_static);
        menu_texts->snake_game_text_static.text_rect.x = LOGICAL_WIDTH / 2;
        menu_texts->snake_game_text_static.text_rect.y = LOGICAL_HEIGHT * 0.25f;

        menu_texts->snake_game_text_static.text_rect.x -= menu_texts->snake_game_text_static.text_rect.w / 2;
        menu_texts->snake_game_text_static.text_rect.y -= menu_texts->snake_game_text_static.text_rect.h / 2;
    }

    {  // Render Start
        // TODO: should we add the should_update to all text calls?
        draw_text_static_2(&menu_texts->start_game_text_static);
        menu_texts->start_game_text_static.text_rect.x = LOGICAL_WIDTH * 0.25f;
        menu_texts->start_game_text_static.text_rect.y = LOGICAL_HEIGHT * 0.75f;

        menu_texts->start_game_text_static.text_rect.x -= menu_texts->start_game_text_static.text_rect.w / 2;
        menu_texts->start_game_text_static.text_rect.y -= menu_texts->start_game_text_static.text_rect.h / 2;
    }

    {  // Render Exit
        draw_text_static_2(&menu_texts->exit_game_text_static);
        menu_texts->exit_game_text_static.text_rect.x = LOGICAL_WIDTH * 0.75f;
        menu_texts->exit_game_text_static.text_rect.y = LOGICAL_HEIGHT * 0.75f;

        menu_texts->exit_game_text_static.text_rect.x -= menu_texts->exit_game_text_static.text_rect.w / 2;
        menu_texts->exit_game_text_static.text_rect.y -= menu_texts->exit_game_text_static.text_rect.h / 2;
    }
}