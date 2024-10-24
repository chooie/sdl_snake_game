#include <iostream>
#include <SDL2/SDL.h>

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

void
render()
{
    // Calculate the size of the square to maintain the aspect ratio
    int square_size = (window_width < window_height) ? window_width / 4 : window_height / 4;

    // Calculate position to center the square
    int square_x = (window_width - square_size) / 2;
    int square_y = (window_height - square_size) / 2;

    // Clear the screen
    SDL_SetRenderDrawColor(global_renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(global_renderer);

    // Set the color for the square
    SDL_SetRenderDrawColor(global_renderer, 255, 0, 0, 255); // Red square

    // Create and draw the square
    SDL_Rect square = { square_x, square_y, square_size, square_size };
    SDL_RenderFillRect(global_renderer, &square);

    // Present the rendered content
    SDL_RenderPresent(global_renderer);
}

int filterEvent(void *userdata, SDL_Event * event) {
    if (event->type == SDL_WINDOWEVENT && (event->window.event == SDL_WINDOWEVENT_RESIZED))
    {
        global_did_resize = 1;
        SDL_GetWindowSize(global_window, &window_width, &window_height);
        // fprintf(stderr, "Width: %d, Height: %d\n", window_width, window_height);
        render();

        // Take the event off the internal queue
        return 0;
    }

    // Not an event we need to immediately process, so allow it to remain on the queue
    return 1;
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);
    
    // const char* platform = SDL_GetPlatform();
    // std::cout << "Platform " << platform << std::endl;

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

    SDL_Event event;

    SDL_SetEventFilter(filterEvent, &event);

    srand((unsigned int)time(NULL));
    
    Uint64 performance_frequency = SDL_GetPerformanceFrequency();
    Uint64 timer_last_frame_drawn = SDL_GetPerformanceCounter();
    unsigned int counter = 0;

    while (global_running)
    {
        Uint64 timer_now = SDL_GetPerformanceCounter();
        float time_delta_ms = ((float)(timer_now - timer_last_frame_drawn) / (float)performance_frequency) * 1000;

        if (time_delta_ms >= TARGET_TIME_PER_FRAME_MS)
        {
            float fps = 1000.0f / time_delta_ms;
            std::cout << "fps: " << fps << std::endl;

            counter++;

            if (counter >= (unsigned int)TARGET_SCREEN_FPS) {
                counter = 0;

                char fps_str[20];  // Allocate enough space for the string
                sprintf(fps_str, "%.2f", fps);

                // Generate a random number in a range (for example, between 0 and 100)
                int random_number = rand() % 101; // Generates a number between 0 and 100
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

                for (int i = 0; fps_str[i] != '\0'; i++) {
                    title_str[index_to_start_adding++] = fps_str[i];
                }

                title_str[index_to_start_adding++] = ')';

                title_str[index_to_start_adding++] = '\0';

                SDL_SetWindowTitle(global_window, title_str);
            }

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
                render();
            } else {
                global_did_resize = 0;
            }

            timer_last_frame_drawn = SDL_GetPerformanceCounter();
            float time_elapsed_this_frame_ms = ((float)(timer_last_frame_drawn - timer_now) / (float)performance_frequency) * 1000;

            if (time_elapsed_this_frame_ms < TARGET_TIME_PER_FRAME_MS) {
                SDL_Delay((int)(TARGET_TIME_PER_FRAME_MS - time_elapsed_this_frame_ms));
            } else {
                printf("Missed frame!");
            }
        }
    }

    SDL_DestroyRenderer(global_renderer);
    SDL_DestroyWindow(global_window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
