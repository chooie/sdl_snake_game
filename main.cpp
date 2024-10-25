#include "common.h"

#include <iostream>
#include <SDL2/SDL.h>

#ifdef __WINDOWS__
#include <windows.h>
#include <mmsystem.h>
#endif

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
int32 global_did_resize = 0;
SDL_Rect global_viewport;

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
    SDL_Rect square = { 0, 0, size, size };  // The square fills the texture
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

    SDL_Rect dst_rect = { square_x, square_y, square_size, square_size };

    // Clear the screen
    SDL_SetRenderDrawColor(global_renderer, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(global_renderer);

    // Center of the square for rotation
    SDL_Point center = { square_size / 2, square_size / 2 };

    // Render the rotating square using SDL_RenderCopyEx
    SDL_RenderCopyEx(global_renderer, square_texture, nullptr, &dst_rect, angle, &center, SDL_FLIP_NONE);

    // Present the rendered content
    SDL_RenderPresent(global_renderer);
}

int32 filterEvent(void* userdata, SDL_Event* event) {
    if (event->type == SDL_WINDOWEVENT) {
        if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
            global_did_resize = 1;
            return 0; // Prevent excessive rendering during resize
        }
        // Ignore other window events during drag
        if (event->window.event == SDL_WINDOWEVENT_MOVED) {
            return 0;
        }
    }
    return 1; // Allow other events
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

    global_window = SDL_CreateWindow("SDL Starter",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     LOGICAL_WIDTH, LOGICAL_HEIGHT,
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
    /*| SDL_RENDERER_PRESENTVSYNC*/);
    if (!global_renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(global_window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Texture* square_texture = createSquareTexture(global_renderer, window_width / 4);  // Create the texture for the square

    SDL_Event event;

    SDL_SetEventFilter(filterEvent, &event);

    Uint64 PERFORMANCE_FREQUENCY = SDL_GetPerformanceFrequency();
    Uint64 counter_start_frame = SDL_GetPerformanceCounter();
    // Sometimes we're going to oversleep, so we need to account for that potentially
    real32 frame_time_debt_s = 0;
    unsigned int counter = 0;

    real32 angle = 0.0f;  // Rotation angle
    real32 dt_s = 0;
    Uint64 counter_before = SDL_GetPerformanceCounter();

    while (global_running)
    {
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
                {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                        {
                            global_running = false;
                        } break;
                        case SDLK_s:
                        {
                            SDL_WarpMouseInWindow(global_window, window_width / 2, window_height / 2);
                        } break;
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
                } break;

                case SDL_WINDOWEVENT:
                {
                    switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_CLOSE: 
                        {
                            global_running = false;
                        } break;
                    }
                } break;

                case SDL_QUIT:
                {
                    global_running = false;
                    break;
                }
            }
        }

        if (!global_did_resize) {
            Uint64 counter_now = SDL_GetPerformanceCounter();
            dt_s = ((real32)(counter_now - counter_before) / (real32)PERFORMANCE_FREQUENCY);
            angle += 90.0f * dt_s;  // Rotate 90 degrees per second
            // Render the rotating square with the current angle
            render(square_texture, angle);
            counter_before = counter_now;
        } else {
            SDL_GetWindowSize(global_window, &window_width, &window_height);
            global_did_resize = 0;
        }

        Uint64 counter_end_frame = SDL_GetPerformanceCounter();

        real32 frame_time_elapsed_s = ((real32)(counter_end_frame - counter_start_frame) / (real32)PERFORMANCE_FREQUENCY);

        real32 sleep_time_s = (TARGET_TIME_PER_FRAME_S - frame_time_debt_s) - frame_time_elapsed_s;
        if (sleep_time_s > 0) {
            real32 sleep_time_ms = sleep_time_s * 1000.0f;
            SDL_Delay((uint32)sleep_time_ms);
        } else {
            printf("Missed frame!\n");
        }
        Uint64 counter_after_sleep = SDL_GetPerformanceCounter();
        real32 actual_frame_time_s = ((real32)(counter_after_sleep - counter_start_frame) / (real32)PERFORMANCE_FREQUENCY);
        
        frame_time_debt_s = actual_frame_time_s - TARGET_TIME_PER_FRAME_S;
        if (frame_time_debt_s < 0) {
            frame_time_debt_s = 0;
        }

        real32 fps = 1.0f / actual_frame_time_s;
        // std::cout << "fps: " << fps << std::endl;

        counter++;
        if (counter >= (uint32)TARGET_SCREEN_FPS) {
            counter = 0;

            char fps_str[20];  // Allocate enough space for the string
            sprintf(fps_str, "%.2f", fps);

            // Generate a random number in a range (for example, between 0 and 100)
            int32 random_number = rand() % 101; // Generates a number between 0 and 100
            char random_str[20];
            sprintf(random_str, "%d", random_number);

            char title_str[100] = "SDL Starter (FPS: ";
            int index_to_start_adding = 0;
            while (title_str[index_to_start_adding] != '\0') {
                index_to_start_adding++;
            }

            // for (int i = 0; random_str[i] != '\0'; i++) {
            //     title_str[index_to_start_adding++] = random_str[i];
            // }

            for (uint32 i = 0; fps_str[i] != '\0'; i++) {
                title_str[index_to_start_adding++] = fps_str[i];
            }

            title_str[index_to_start_adding++] = ')';

            title_str[index_to_start_adding++] = '\0';

            SDL_SetWindowTitle(global_window, title_str);
        }

        counter_start_frame = counter_after_sleep;
    }

    SDL_DestroyRenderer(global_renderer);
    SDL_DestroyWindow(global_window);
    SDL_Quit();

    return EXIT_SUCCESS;
}