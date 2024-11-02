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

#include <chrono>

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

real32 SIMULATION_FPS = 100;
real32 SIMULATION_DELTA_TIME_S = 1.f / SIMULATION_FPS;

real32 TARGET_SCREEN_FPS = 59.9f;
real32 TARGET_TIME_PER_FRAME_S = 1.f / (real32)TARGET_SCREEN_FPS;
real32 TARGET_TIME_PER_FRAME_MS = 1000.f / (real32)TARGET_SCREEN_FPS;

int32 global_running = 1;
SDL_Window* global_window;
SDL_Renderer* global_renderer;
Uint64 GLOBAL_PERFORMANCE_FREQUENCY;
bool32 global_paused = 0;
Uint64 global_counter_last_frame;

TTF_Font* global_font;
TTF_Font* global_debug_font;
SDL_Rect global_text_rect;
SDL_Surface* global_text_surface;
SDL_Texture* global_text_texture;

real32 global_debug_counter;
Uint64 global_tick_counter_before;

struct Master_Timer {
    // High-res timer stuff
    Uint64 last_frame_counter;
    Uint64 COUNTER_FREQUENCY;

    real32 frame_time_elapsed_before_sleep__seconds; // Useful to see how much time was really needed
    real32 total_frame_time_elapsed__seconds; // After the sleep or vsync block

    real64 physics_simluation_elapsed_time__seconds;
};

#ifdef __WIN32__
uint64 global_last_cycle_count;
uint64 global_cycles_elapsed_without_delay;
uint64 global_total_cycles_elapsed;
#endif

real32 global_frame_time_debt_s;
void update_timer(Master_Timer* t, bool32 vsync_enabled)
{
#ifdef __WIN32__
    uint64 global_cycle_count_now = __rdtsc();
    global_cycles_elapsed_without_delay = global_cycle_count_now - global_last_cycle_count;
#endif

    Uint64 counter_now = SDL_GetPerformanceCounter();

    t->frame_time_elapsed_before_sleep__seconds =
        ((real32)(counter_now - t->last_frame_counter) / (real32)t->COUNTER_FREQUENCY);

    if (!vsync_enabled)
    {
        real32 sleep_time_s =
            (TARGET_TIME_PER_FRAME_S - global_frame_time_debt_s) - t->frame_time_elapsed_before_sleep__seconds;

        if (sleep_time_s > 0)
        {
            real32 sleep_time_ms = sleep_time_s * 1000.0f;
            // printf("Sleep ms: %.2f\n", sleep_time_ms);

            // Round sleep time up
            SDL_Delay((uint32)(sleep_time_ms + 0.5f));
        }
        else
        {
            // TODO: logging when we miss a frame?
        }
    }

#ifdef __WIN32__
    uint64 global_end_cycle_count_after_delay = __rdtsc();
    global_total_cycles_elapsed = global_end_cycle_count_after_delay - global_last_cycle_count;
#endif

    Uint64 counter_after_sleep = SDL_GetPerformanceCounter();
    t->total_frame_time_elapsed__seconds =
        ((real32)(counter_after_sleep - t->last_frame_counter) / (real32)t->COUNTER_FREQUENCY);

    // printf("Frame time: %.2f\n", t->total_frame_time_elapsed__seconds * 1000.f);

    if (!vsync_enabled)
    {
        // global_frame_time_debt_s = t->total_frame_time_elapsed__seconds - TARGET_TIME_PER_FRAME_S;

        // if (global_frame_time_debt_s < 0)
        // {
        //     global_frame_time_debt_s = 0;
        // }
    }

    // Next iteration
    t->last_frame_counter = counter_after_sleep;

#ifdef __WIN32__
    global_last_cycle_count = global_cycle_count_now;
#endif
}

SDL_Texture* global_square_texture;

SDL_Texture* createSquareTexture(SDL_Renderer* renderer, int32 size)
{
    // Create an SDL texture to represent the square
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size, size);

    // Set the texture as the rendering target
    SDL_SetRenderTarget(renderer, texture);

    // Clear the texture (make it transparent)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);  // Fully transparent
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 247, 202, 136, 255);
    SDL_Rect square = {0, 0, size, size};              // The square fills the texture
    SDL_RenderFillRect(renderer, &square);

    // Reset the render target back to the default window
    SDL_SetRenderTarget(renderer, nullptr);

    return texture;
}

#include "input.cpp"
#include "game.cpp"
#include "render.cpp"

int32 filterEvent(void* userdata, SDL_Event* event)
{
    Input* input = (Input*)(userdata);

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
                                     SDL_WINDOW_ALLOW_HIGHDPI |SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if (!global_window)
    {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    global_renderer = SDL_CreateRenderer(global_window, -1, SDL_RENDERER_ACCELERATED);
    if (!global_renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(global_window);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_RenderSetLogicalSize(global_renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);
    // Set linear scaling for smoother scaling
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");


    global_font = TTF_OpenFont("fonts/Roboto/Roboto-Medium.ttf", 256);
    if (global_font == nullptr) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return -1;
    }

    global_debug_font = TTF_OpenFont("fonts/Roboto/Roboto-Medium.ttf", 16);
    if (global_debug_font == nullptr) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return -1;
    }


    SDL_Color text_color = {255, 255, 255};  // White color
    const char* text = "Help! I'm trapped in some empty hellscape.";
    global_text_surface = TTF_RenderText_Blended(global_font, text, text_color);
    global_text_texture = SDL_CreateTextureFromSurface(global_renderer, global_text_surface);
    SDL_FreeSurface(global_text_surface);

    global_text_rect = {};
    // TTF_SetFontSize(global_font, 512);
    TTF_SizeText(global_font, text, &global_text_rect.w, &global_text_rect.h);

    real32 desired_width = LOGICAL_WIDTH * 0.9f;
    real32 text_aspect_ratio = (real32)global_text_rect.w / (real32)global_text_rect.h;
    global_text_rect.w = (int32)desired_width;

    global_text_rect.h = int32(desired_width / text_aspect_ratio);

    global_text_rect.x = (LOGICAL_WIDTH / 2) - global_text_rect.w / 2;
    global_text_rect.y = (LOGICAL_HEIGHT / 2) - global_text_rect.h / 2;

    // SDL_QueryTexture(global_text_texture, nullptr, nullptr, &global_text_rect.w, &global_text_rect.h);

    global_square_texture = createSquareTexture(global_renderer, window_width / 4);

    SDL_Event event;
    Input input = {};

    SDL_SetEventFilter(filterEvent, &input);

    GLOBAL_PERFORMANCE_FREQUENCY = SDL_GetPerformanceFrequency();

    Master_Timer master_timer = {};
    master_timer.COUNTER_FREQUENCY = GLOBAL_PERFORMANCE_FREQUENCY;
    master_timer.last_frame_counter = SDL_GetPerformanceCounter();

    global_frame_time_debt_s = 0;

    global_debug_counter = 0;
    global_tick_counter_before = SDL_GetPerformanceCounter();

    global_counter_last_frame = SDL_GetPerformanceCounter();

    real32 accumulator_s = 0.0f;

    State previous_state = {};
    State current_state = {};
    
    #ifdef __WIN32__
    // This looks like a function call but it's actually an intrinsic that
    // runs the actual assembly instruction directly
    global_last_cycle_count = __rdtsc();
    #endif

    char debug_text[100] = "";
    char debug_text_2[100] = "";

    bool32 vsync_enabled = 1;
#ifdef __APPLE__
    vsync_enabled = 1;
#endif

    SDL_RenderSetVSync(global_renderer, vsync_enabled);

    while (global_running)
    {
        // Clear the screen
        SDL_SetRenderDrawColor(global_renderer, 0, 0, 0, 255);  // Black background
        SDL_RenderClear(global_renderer);

        update_timer(&master_timer, vsync_enabled);
        // printf("Time elapsed: %.2f\n", master_timer.physics_simluation_elapsed_time__seconds);
        handle_input(&event, &input);

        if (global_paused)
        {
            global_counter_last_frame = SDL_GetPerformanceCounter();
        }

        if (!global_paused)
        {
            // https://gafferongames.com/post/fix_your_timestep/
            real32 frame_time_s = master_timer.total_frame_time_elapsed__seconds;

            if (frame_time_s > 0.25f)
            {
                // Prevent "spiraling" (exessive frame accumulation) in case of a big lag spike.
                frame_time_s = 0.25f;
            }

            accumulator_s += frame_time_s;

            while (accumulator_s >= SIMULATION_DELTA_TIME_S)
            {  // Simulation 'consumes' whatever time is given to it based on the render rate
                previous_state = current_state;
                simulate(
                    &current_state,
                    &input,
                    master_timer.physics_simluation_elapsed_time__seconds,
                    SIMULATION_DELTA_TIME_S
                );
                master_timer.physics_simluation_elapsed_time__seconds += SIMULATION_DELTA_TIME_S;
                accumulator_s -= SIMULATION_DELTA_TIME_S;

                // printf("Timer: %.2f\n", simulation_time_elapsed_s);
            }
        }

        real32 alpha = accumulator_s / SIMULATION_DELTA_TIME_S;
        // Interpolate between the current state and previous state
        // NOTE: the render always lags by about a frame
        State state = current_state * alpha + previous_state * (1.0f - alpha);

        render(&state, global_square_texture);

#if 0
        // Eat CPU time to test debug stuff
        auto start = std::chrono::high_resolution_clock::now();
    
        // Run a loop that consumes CPU cycles for 10 seconds
        while (true) {
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            
            if (duration.count() >= 4.0f) {
                break;
            }
        }
#endif

#ifdef __WIN32__
        real64 mega_cycles_for_actual_work = global_cycles_elapsed_without_delay / (1000.0f * 1000.0f);
        real64 mega_cycles_per_frame = global_total_cycles_elapsed / (1000.0f * 1000.0f);
#endif

        SDL_RenderCopy(global_renderer, global_text_texture, nullptr, &global_text_rect);

        // Disable logical size scaling temporarily
        SDL_RenderSetLogicalSize(global_renderer, 0, 0);

        SDL_Color debug_text_color = {255, 255, 255};  // White color

        if (global_debug_counter == 0)
        {
#ifdef __WIN32__
            snprintf(debug_text, sizeof(debug_text), "Mega cycles/Frame: %.02f", mega_cycles_for_actual_work);
#endif
        }

        real32 y_offset = 0;
        real32 padding = 5.0f;

        SDL_Surface* debug_text_surface = TTF_RenderText_Blended(global_debug_font, debug_text, debug_text_color);
        SDL_Texture* debug_text_texture = SDL_CreateTextureFromSurface(global_renderer, debug_text_surface);
        SDL_FreeSurface(debug_text_surface);

        SDL_Rect debug_text_rect = {};

        // TTF_SetFontSize(global_font, 512);
        TTF_SizeText(global_debug_font, debug_text, &debug_text_rect.w, &debug_text_rect.h);

        debug_text_rect.x = (int32)(LOGICAL_WIDTH * 0.01f);
        debug_text_rect.y = (int32)(LOGICAL_HEIGHT * 0.01f);

        SDL_RenderCopy(global_renderer, debug_text_texture, NULL, &debug_text_rect);

        y_offset += debug_text_rect.h + padding;

        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

        real32 fps = 1.0f / master_timer.total_frame_time_elapsed__seconds;

        if (global_debug_counter == 0)
        {
#ifdef __WIN32__
            snprintf(debug_text_2, sizeof(debug_text_2), "FPS: %.02f", fps);
#endif
        }

        SDL_Surface* debug_text_surface_2 = TTF_RenderText_Blended(global_debug_font, debug_text_2, debug_text_color);
        SDL_Texture* debug_text_texture_2 = SDL_CreateTextureFromSurface(global_renderer, debug_text_surface_2);
        SDL_FreeSurface(debug_text_surface_2);

        SDL_Rect debug_text_rect_2 = {};

        // TTF_SetFontSize(global_font, 512);
        TTF_SizeText(global_debug_font, debug_text_2, &debug_text_rect_2.w, &debug_text_rect_2.h);

        debug_text_rect_2.x = (int32)(LOGICAL_WIDTH * 0.01f);
        debug_text_rect_2.y = (int32)y_offset + (int32)(LOGICAL_HEIGHT * 0.01f);

        SDL_RenderCopy(global_renderer, debug_text_texture_2, NULL, &debug_text_rect_2);

        // Re-enable logical size scaling for other elements
        SDL_RenderSetLogicalSize(global_renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);

        y_offset += debug_text_rect.h + padding;


#if 0
        if (global_debug_counter == 0)
        {
            char fps_str[30];  // Allocate enough space for the string
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
#endif

        global_debug_counter += master_timer.total_frame_time_elapsed__seconds;

        // Tick every second
        if (global_debug_counter >= 1.0f)
        {
            global_debug_counter = 0;
        }

        // Present the rendered content
        SDL_RenderPresent(global_renderer);
        SDL_SetRenderTarget(global_renderer, NULL);
    }

    TTF_CloseFont(global_font);
    TTF_CloseFont(global_debug_font);
    SDL_DestroyRenderer(global_renderer);
    SDL_DestroyWindow(global_window);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}