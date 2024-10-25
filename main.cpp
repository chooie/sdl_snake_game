// clang-format off
#include "common.h"

#include <iostream>
#include <SDL2/SDL.h>

#ifdef __WINDOWS__
#include <windows.h>
#include <mmsystem.h>
#endif
// clang-format on

int32 LOGICAL_WIDTH = 1280;
int32 LOGICAL_HEIGHT = 720;
real32 ABSOLUTE_ASPECT_RATIO = real32(LOGICAL_WIDTH) / (real32)LOGICAL_HEIGHT;

int32 window_width = LOGICAL_WIDTH;
int32 window_height = LOGICAL_HEIGHT;

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

        // Generate a random number in a range (for example, between 0 and 100)
        int32 random_number = rand() % 101;  // Generates a number between 0 and 100
        char random_str[20];
        sprintf(random_str, "%d", random_number);

        char title_str[100] = "SDL Starter (FPS: ";
        int index_to_start_adding = 0;
        while (title_str[index_to_start_adding] != '\0')
        {
            index_to_start_adding++;
        }

        // for (int i = 0; random_str[i] != '\0'; i++) {
        //     title_str[index_to_start_adding++] = random_str[i];
        // }

        for (uint32 i = 0; fps_str[i] != '\0'; i++)
        {
            title_str[index_to_start_adding++] = fps_str[i];
        }

        title_str[index_to_start_adding++] = ')';

        title_str[index_to_start_adding++] = '\0';

        SDL_SetWindowTitle(global_window, title_str);
    }

    global_counter_start_frame = counter_after_sleep;
}

SDL_Texture* createSquareTexture(SDL_Renderer* renderer, int32 size)
{
    // Create an SDL texture to represent the square
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size, size);

    // Set the texture as the rendering target
    SDL_SetRenderTarget(renderer, texture);

    // Clear the texture (make it transparent)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);  // Fully transparent
    SDL_RenderClear(renderer);

    // Set the color of the square (red)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red color
    SDL_Rect square = {0, 0, size, size};              // The square fills the texture
    SDL_RenderFillRect(renderer, &square);

    // Reset the render target back to the default window
    SDL_SetRenderTarget(renderer, nullptr);

    return texture;
}

void render(SDL_Texture* square_texture, real32 angle)
{
    // Calculate the size of the square
    int32 square_size = (window_width < window_height) ? window_width / 4 : window_height / 4;

    // Calculate position to center the square
    int32 square_x = (window_width - square_size) / 2;
    int32 square_y = (window_height - square_size) / 2;

    SDL_Rect dst_rect = {square_x, square_y, square_size, square_size};

    // Clear the screen
    SDL_SetRenderDrawColor(global_renderer, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(global_renderer);

    // Center of the square for rotation
    SDL_Point center = {square_size / 2, square_size / 2};

    // Render the rotating square using SDL_RenderCopyEx
    SDL_RenderCopyEx(global_renderer, square_texture, nullptr, &dst_rect, angle, &center, SDL_FLIP_NONE);

    // Present the rendered content
    SDL_RenderPresent(global_renderer);
}

SDL_Texture* global_square_texture;
real32 global_angle;
real32 global_dt_s;
Uint64 global_counter_before;

void main_work()
{
    Uint64 counter_now = SDL_GetPerformanceCounter();
    global_dt_s = ((real32)(counter_now - global_counter_before) / (real32)GLOBAL_PERFORMANCE_FREQUENCY);
    global_angle += 90.0f * global_dt_s;  // Rotate 90 degrees per second
    // Render the rotating square with the current angle
    render(global_square_texture, global_angle);
    global_counter_before = counter_now;

    limit_fps();
}

int32 filterEvent(void* userdata, SDL_Event* event)
{
    if (event->type == SDL_WINDOWEVENT)
    {
        if (event->window.event == SDL_WINDOWEVENT_RESIZED)
        {
            SDL_GetWindowSize(global_window, &window_width, &window_height);
        }

        if (event->window.event == SDL_WINDOWEVENT_RESIZED || event->window.event == SDL_WINDOWEVENT_EXPOSED)
        {
            main_work();
        }
    }
    return 1;  // Allow other events
}

int32 main(int32 argc, char* argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    // const char* platform = SDL_GetPlatform();
    // std::cout << "Platform " << platform << std::endl;

#ifdef __WINDOWS__
    timeBeginPeriod(1);
#endif

// Set macOS-specific hint
#ifdef __APPLE__
    SDL_SetHint(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, "1");
#endif

    global_window = SDL_CreateWindow(
        "SDL Starter",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        LOGICAL_WIDTH,
        LOGICAL_HEIGHT,
        // TODO: how do I make stuff scale for high dpi devices?
        /*SDL_WINDOW_ALLOW_HIGHDPI | */
        SDL_WINDOW_RESIZABLE
    );
    if (!global_window)
    {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    global_renderer = SDL_CreateRenderer(
        global_window, -1, SDL_RENDERER_ACCELERATED
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

    global_square_texture =
        createSquareTexture(global_renderer, window_width / 4);  // Create the texture for the square

    SDL_Event event;

    SDL_SetEventFilter(filterEvent, &event);

    GLOBAL_PERFORMANCE_FREQUENCY = SDL_GetPerformanceFrequency();

    global_counter_start_frame = SDL_GetPerformanceCounter();
    global_frame_time_debt_s = 0;
    global_counter = 0;

    global_angle = 0.0f;  // Rotation angle
    global_dt_s = 0;
    global_counter_before = SDL_GetPerformanceCounter();

    while (global_running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                        {
                            global_running = false;
                        }
                        break;
                        case SDLK_s:
                        {
                            SDL_WarpMouseInWindow(global_window, window_width / 2, window_height / 2);
                        }
                        break;
                        case SDLK_f:
                        {
                            int isFullScreen = SDL_GetWindowFlags(global_window) & SDL_WINDOW_FULLSCREEN;
                            if (isFullScreen)
                            {
                                SDL_SetWindowFullscreen(global_window, 0);
                            }
                            else
                            {
                                SDL_SetWindowFullscreen(global_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
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
                            global_running = false;
                        }
                        break;
                    }
                }
                break;

                case SDL_QUIT:
                {
                    global_running = false;
                    break;
                }
            }
        }

        main_work();
    }

    SDL_DestroyRenderer(global_renderer);
    SDL_DestroyWindow(global_window);
    SDL_Quit();

    return EXIT_SUCCESS;
}