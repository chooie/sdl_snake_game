#include <SDL2/SDL.h>

struct Menu_Texts
{
    Drawn_Text_Static snake_game_text_static;
    Drawn_Text_Static_2 start_game_text_static;
};

struct Start_Screen__State
{
    Menu_Texts* menu_texts;
};

Menu_Texts start_screen__setup_text()
{
    SDL_Color white_text_color = { 255, 255, 255, 255 }; // White color
    real32 font_size = 16.0f;

    Drawn_Text_Static snake_game_text_static = {};
    {
        snake_game_text_static.text_string = "Snake Game";
    }
    snake_game_text_static.font_size = font_size * 2.f;
    snake_game_text_static.color = white_text_color;
    snake_game_text_static.text_rect.x = -LOGICAL_WIDTH; // Draw off-screen initially;

    Drawn_Text_Static_2 start_game_text_static = {};
    {
        start_game_text_static.text_string = "Start";
    }
    start_game_text_static.font_size = font_size * 1.5f;
    start_game_text_static.color = white_text_color;
    start_game_text_static.text_rect.x = -LOGICAL_WIDTH; // Draw off-screen initially;

    Menu_Texts menu_texts = {};
    menu_texts.snake_game_text_static = snake_game_text_static;
    menu_texts.start_game_text_static = start_game_text_static;

    return menu_texts;
}

void start_screen__handle_input(Scene* scene, Input* input)
{
    Start_Screen__State* state = (Start_Screen__State*)scene->state;
    if (pressed(BUTTON_ENTER))
    {
        global_next_scene = &global_gameplay_scene;
    }
}

int32 global_time_before = -1;
void start_screen__update(struct Scene* scene,
                          Audio_Context* audio_context,
                          real64 simulation_time_elapsed,
                          real32 dt_s)
{
    if ((int32)simulation_time_elapsed != global_time_before)
    {
        global_time_before = simulation_time_elapsed;
        printf("Hey\n");
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
        menu_texts->snake_game_text_static.text_rect.y = LOGICAL_HEIGHT / 2;

        menu_texts->snake_game_text_static.text_rect.x -=
            menu_texts->snake_game_text_static.text_rect.w / 2;
        menu_texts->snake_game_text_static.text_rect.y -=
            menu_texts->snake_game_text_static.text_rect.h / 2;
    }

    {  // Render Snake Game
        draw_text_static_2(&menu_texts->start_game_text_static);
        menu_texts->start_game_text_static.text_rect.x = LOGICAL_WIDTH / 2;
        menu_texts->start_game_text_static.text_rect.y = LOGICAL_HEIGHT * 0.75f;

        menu_texts->start_game_text_static.text_rect.x -=
            menu_texts->start_game_text_static.text_rect.w / 2;
        menu_texts->start_game_text_static.text_rect.y -=
            menu_texts->start_game_text_static.text_rect.h / 2;
    }
}