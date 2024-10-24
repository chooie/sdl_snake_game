#include <iostream>
#include <SDL2/SDL.h>

#ifdef __WINDOWS__
#include <windows.h>
#include <mmsystem.h>
#endif

int LOGICAL_WIDTH = 1280;
int LOGICAL_HEIGHT = 720;
float ABSOLUTE_ASPECT_RATIO = float(LOGICAL_WIDTH) / (float)LOGICAL_HEIGHT;

int window_width = LOGICAL_WIDTH;
int window_height = LOGICAL_HEIGHT;

int TARGET_SCREEN_FPS = 60;
float TARGET_TIME_PER_FRAME_MS = 1000.f / (float)TARGET_SCREEN_FPS;

int global_running = 1;
SDL_Window* global_window;
SDL_Renderer* global_renderer;
int global_did_resize = 0;
SDL_Rect global_viewport;

SDL_Texture* createSquareTexture(SDL_Renderer* renderer, int size)
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

void render(SDL_Texture* square_texture, float angle)
{
    // Calculate the size of the square
    int square_size = (window_width < window_height) ? window_width / 4 : window_height / 4;

    // Calculate position to center the square
    int square_x = (window_width - square_size) / 2;
    int square_y = (window_height - square_size) / 2;

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

int filterEvent(void *userdata, SDL_Event *event) {
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

int main(int argc, char* argv[])
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

    Uint64 performance_frequency = SDL_GetPerformanceFrequency();
    Uint64 timer_last_frame_drawn = SDL_GetPerformanceCounter();
    unsigned int counter = 0;

    float angle = 0.0f;  // Rotation angle
    float dt_s = 0;
    Uint64 counter_before = SDL_GetPerformanceCounter();

    while (global_running)
    {
        Uint64 timer_now = SDL_GetPerformanceCounter();
        float time_delta_ms = ((float)(timer_now - timer_last_frame_drawn) / (float)performance_frequency) * 1000;

        if (time_delta_ms >= TARGET_TIME_PER_FRAME_MS)
        {
            float fps = 1000.0f / time_delta_ms;
            std::cout << "fps: " << fps << std::endl;

            counter++;

            if (counter >= (unsigned int)TARGET_SCREEN_FPS)
            {
                counter = 0;

                char fps_str[20]; // Allocate enough space for the string
                sprintf(fps_str, "%.2f", fps);

                // Generate a random number in a range (for example, between 0 and 100)
                int random_number = rand() % 101; // Generates a number between 0 and 100
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

                for (int i = 0; fps_str[i] != '\0'; i++)
                {
                    title_str[index_to_start_adding++] = fps_str[i];
                }

                title_str[index_to_start_adding++] = ')';

                title_str[index_to_start_adding++] = '\0';

                SDL_SetWindowTitle(global_window, title_str);
            }

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

            if (!global_did_resize)
            {
                Uint64 counter_now = SDL_GetPerformanceCounter();
                dt_s = ((float)(counter_now - counter_before) / (float)performance_frequency);
                printf("%f\n", dt_s);
                angle += 90.0f * dt_s; // Rotate 90 degrees per second
                // Render the rotating square with the current angle
                render(square_texture, angle);
                counter_before = counter_now;
            }
            else
            {
                global_did_resize = 0;
            }

            timer_last_frame_drawn = SDL_GetPerformanceCounter();
            float time_elapsed_this_frame_ms = ((float)(timer_last_frame_drawn - timer_now) / (float)performance_frequency) * 1000;

            if (time_elapsed_this_frame_ms < TARGET_TIME_PER_FRAME_MS)
            {
                SDL_Delay((int)(TARGET_TIME_PER_FRAME_MS - time_elapsed_this_frame_ms));
            }
            else
            {
                printf("Missed frame!");
            }
        }
    }

    SDL_DestroyRenderer(global_renderer);
    SDL_DestroyWindow(global_window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
