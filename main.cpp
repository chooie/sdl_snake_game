// clang-format off
#include "common.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#ifdef __WINDOWS__
#include <windows.h>
#include <mmsystem.h>
#endif
// clang-format on

struct Button_State
{
    bool32 is_down;
    bool32 changed;
};

enum
{
    BUTTON_W,
    BUTTON_A,
    BUTTON_S,
    BUTTON_D,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_ENTER,

    BUTTON_SPACE,

    BUTTON_COUNT,  // Should be the last item
};

struct Input
{
    Button_State buttons[BUTTON_COUNT];
};

int32 LOGICAL_WIDTH = 1280;
int32 LOGICAL_HEIGHT = 720;
real32 ABSOLUTE_ASPECT_RATIO = (real32)LOGICAL_WIDTH / (real32)LOGICAL_HEIGHT;

int32 window_width = LOGICAL_WIDTH;
int32 window_height = LOGICAL_HEIGHT;

real32 SIMULATION_DELTA_TIME_S = 0.01f;

int32 TARGET_SCREEN_FPS = 60;
real32 TARGET_TIME_PER_FRAME_S = 1.f / (real32)TARGET_SCREEN_FPS;
real32 TARGET_TIME_PER_FRAME_MS = 1000.f / (real32)TARGET_SCREEN_FPS;

int32 global_running = 1;
SDL_Window* global_window;
SDL_Renderer* global_renderer;
Uint64 GLOBAL_PERFORMANCE_FREQUENCY;
Uint64 global_counter_start_frame;
// Sometimes we're going to oversleep, so we need to account for that potentially
real32 global_frame_time_debt_s;
uint32 global_counter;
bool32 global_paused = 0;
Uint64 global_counter_last_frame;
TTF_Font* global_font;
SDL_Rect global_text_rect;
SDL_Surface* global_text_surface;
SDL_Texture* global_text_texture;

void limit_fps()
{
    Uint64 counter_end_frame = SDL_GetPerformanceCounter();

    real32 frame_time_elapsed_s =
        ((real32)(counter_end_frame - global_counter_start_frame) / (real32)GLOBAL_PERFORMANCE_FREQUENCY);

    real32 sleep_time_s = (TARGET_TIME_PER_FRAME_S - global_frame_time_debt_s) - frame_time_elapsed_s;
    if (sleep_time_s > 0)
    {
        real32 sleep_time_ms = sleep_time_s * 1000.0f;
        SDL_Delay((uint32)sleep_time_ms);
    }
    else
    {
        // printf("Missed frame!\n");
    }
    Uint64 counter_after_sleep = SDL_GetPerformanceCounter();
    real32 actual_frame_time_s =
        ((real32)(counter_after_sleep - global_counter_start_frame) / (real32)GLOBAL_PERFORMANCE_FREQUENCY);
    // Set this for the next iteration
    global_counter_start_frame = counter_after_sleep;


    global_frame_time_debt_s = actual_frame_time_s - TARGET_TIME_PER_FRAME_S;

    if (global_frame_time_debt_s < 0)
    {
        global_frame_time_debt_s = 0;
    }

    real32 fps = 1.0f / actual_frame_time_s;
    // std::cout << "fps: " << fps << std::endl;

    global_counter++;
    if (global_counter >= (uint32)TARGET_SCREEN_FPS)
    {
        global_counter = 0;

        char fps_str[20];  // Allocate enough space for the string
        sprintf(fps_str, "%.2f", fps);

        char title_str[100] = "SDL Starter (FPS: ";
        int index_to_start_adding = 0;
        while (title_str[index_to_start_adding] != '\0')
        {
            index_to_start_adding++;
        }

        for (uint32 i = 0; fps_str[i] != '\0'; i++)
        {
            title_str[index_to_start_adding++] = fps_str[i];
        }

        title_str[index_to_start_adding++] = ')';

        title_str[index_to_start_adding++] = '\0';

        SDL_SetWindowTitle(global_window, title_str);
    }
}

#define is_down(b) input->buttons[b].is_down
#define pressed(b) input->buttons[b].is_down && input->buttons[b].changed
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

#include "game.cpp"
#include "render.cpp"

int32 filterEvent(void* userdata, SDL_Event* event)
{
    Input* input = static_cast<Input*>(userdata);

    if (event->type == SDL_WINDOWEVENT)
    {
        if (event->window.event == SDL_WINDOWEVENT_RESIZED)
        {
            SDL_GetWindowSize(global_window, &window_width, &window_height);
        }

        if (event->window.event == SDL_WINDOWEVENT_RESIZED || event->window.event == SDL_WINDOWEVENT_EXPOSED)
        {
            // main_work(input);
        }
    }
    return 1;  // Allow other events
}

int32 main(int32 argc, char* argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    if (TTF_Init() == -1) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        return -1;
    }

    // Might need this for antialiasing?
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    // const char* platform = SDL_GetPlatform();
    // std::cout << "Platform " << platform << std::endl;

#ifdef __WINDOWS__
    timeBeginPeriod(1);
#endif

// Set macOS-specific hint
#ifdef __APPLE__
    SDL_SetHint(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, "1");
#endif

    global_window = SDL_CreateWindow("SDL Starter",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     LOGICAL_WIDTH,
                                     LOGICAL_HEIGHT,
                                     // TODO: how do I make stuff scale for high dpi devices?
                                     /*SDL_WINDOW_ALLOW_HIGHDPI | */
                                     SDL_WINDOW_RESIZABLE);
    if (!global_window)
    {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    global_renderer = SDL_CreateRenderer(global_window, -1, SDL_RENDERER_ACCELERATED
                                         // No VSYNC for now as it make moving the window sluggish on MacOS
                                         /*| SDL_RENDERER_PRESENTVSYNC*/
    );
    if (!global_renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(global_window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    global_font = TTF_OpenFont("fonts/Roboto/Roboto-Medium.ttf", 256);
    if (global_font == nullptr) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return -1;
    }

    /*
    SDL_Color text_color = {255, 255, 255};  // White color
    const char* text = "Help! I'm trapped in some empty hellscape.";
    global_text_surface = TTF_RenderText_Blended(global_font, text, text_color);
    global_text_texture = SDL_CreateTextureFromSurface(global_renderer, global_text_surface);
    SDL_FreeSurface(global_text_surface);

    global_text_rect = {};
    global_text_rect.x = 100;
    global_text_rect.y = 100;
    // TTF_SetFontSize(font, 24);
    TTF_SizeText(global_font, text, &global_text_rect.w, &global_text_rect.h);
    // SDL_QueryTexture(global_text_texture, nullptr, nullptr, &global_text_rect.w, &global_text_rect.h);
    */

    SDL_Texture* square_texture = createSquareTexture(global_renderer, window_width / 4);

    SDL_Event event;
    Input input = {};

    SDL_SetEventFilter(filterEvent, &input);

    GLOBAL_PERFORMANCE_FREQUENCY = SDL_GetPerformanceFrequency();

    global_counter_start_frame = SDL_GetPerformanceCounter();
    global_frame_time_debt_s = 0;
    global_counter = 0;

    global_counter_last_frame = SDL_GetPerformanceCounter();

    real64 simulation_time_elapsed_s = 0.0;
    real32 accumulator_s = 0.0f;

    State previous_state = {};
    State current_state = {};

    while (global_running)
    {
        for (int i = 0; i < BUTTON_COUNT; i++)
        {
            input.buttons[i].changed = false;
        }

        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    switch (event.key.keysym.sym)
                    {
#define process_input(button, sdl_key)\
case sdl_key: {\
    if (event.type == SDL_KEYDOWN)\
    {\
        input.buttons[button].changed = input.buttons[button].is_down == 0;\
        input.buttons[button].is_down = 1;\
    } else {\
        input.buttons[button].changed = input.buttons[button].is_down == 1;\
        input.buttons[button].is_down = 0;\
    }\
} break;
                        process_input(BUTTON_W, SDLK_w);
                        process_input(BUTTON_A, SDLK_a);
                        process_input(BUTTON_S, SDLK_s);
                        process_input(BUTTON_D, SDLK_d);
                        process_input(BUTTON_SPACE, SDLK_SPACE);
                    }
                } break;
            }

            switch (event.type)
            {
                case SDL_KEYUP:
                {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                        {
                            global_running = 0;
                        }
                        break;
                        // case SDLK_s:
                        // {
                        //     SDL_WarpMouseInWindow(global_window, window_width / 2, window_height / 2);
                        // } break;
                        case SDLK_f:
                        {
                            int isFullScreen = SDL_GetWindowFlags(global_window) & SDL_WINDOW_FULLSCREEN;
                            if (isFullScreen)
                            {
                                SDL_SetWindowFullscreen(global_window, 0);
                                SDL_ShowCursor(SDL_ENABLE);
                                SDL_SetRelativeMouseMode(SDL_FALSE);
                            }
                            else
                            {
                                SDL_SetWindowFullscreen(global_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                                SDL_ShowCursor(SDL_DISABLE);
                                SDL_SetRelativeMouseMode(SDL_TRUE);
                            }
                        }
                        break;
                    }
                }
                break;

                case SDL_WINDOWEVENT:
                {
                    switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_CLOSE:
                        {
                            global_running = 0;
                        }
                        break;
                    }
                }
                break;

                case SDL_QUIT:
                {
                    global_running = false;
                } break;
            }
        }

        if (input.buttons[BUTTON_SPACE].is_down && input.buttons[BUTTON_SPACE].changed)
        {
            global_paused = !global_paused;
        }

        if (global_paused) {
            global_counter_last_frame = SDL_GetPerformanceCounter();
        }

        if (!global_paused)
        {
            // https://gafferongames.com/post/fix_your_timestep/
            Uint64 counter_now = SDL_GetPerformanceCounter();

            real32 frame_time_s = ((real32)(counter_now - global_counter_last_frame)
                                   /
                                   (real32)GLOBAL_PERFORMANCE_FREQUENCY);
            global_counter_last_frame = counter_now;

            if (frame_time_s > 0.25f)
            {
                // Prevent "spiraling" (exessive frame accumulation) in case of a big lag spike.
                frame_time_s = 0.25f;
            }

            accumulator_s += frame_time_s;

            while (accumulator_s >= SIMULATION_DELTA_TIME_S)
            {   // Simulation 'consumes' whatever time is given to it based on the render rate
                previous_state = current_state;
                simulate(&current_state, &input, simulation_time_elapsed_s, SIMULATION_DELTA_TIME_S);
                simulation_time_elapsed_s += SIMULATION_DELTA_TIME_S;
                accumulator_s -= SIMULATION_DELTA_TIME_S;

                // printf("Timer: %.2f\n", simulation_time_elapsed_s);
            }
        }

        real32 alpha = accumulator_s / SIMULATION_DELTA_TIME_S;
        // Interpolate between the current state and previous state
        // NOTE: the render always lags by about a frame
        State state = current_state * alpha + previous_state * (1.0f - alpha);

        render(&state, square_texture);

        limit_fps();
    }

    SDL_DestroyRenderer(global_renderer);
    SDL_DestroyWindow(global_window);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}