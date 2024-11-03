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

real32 TARGET_SCREEN_FPS = 58.9f;
real32 TARGET_TIME_PER_FRAME_S = 1.f / (real32)TARGET_SCREEN_FPS;
real32 TARGET_TIME_PER_FRAME_MS = 1000.f / (real32)TARGET_SCREEN_FPS;

int32 global_running = 1;
SDL_Window* global_window;
SDL_Renderer* global_renderer;
Uint64 GLOBAL_PERFORMANCE_FREQUENCY;

Uint64 global_counter_last_frame;

TTF_Font* global_font;
TTF_Font* global_debug_font;
SDL_Rect global_text_rect;
SDL_Surface* global_text_surface;
SDL_Texture* global_text_texture;

bool32 global_display_debug_info;
bool32 global_paused;

real32 global_debug_counter;
Uint64 global_tick_counter_before;

struct Master_Timer
{
    // High-res timer stuff
    Uint64 last_frame_counter;
    Uint64 COUNTER_FREQUENCY;

    real32 frame_time_elapsed_before_render__seconds;  // Useful to see how much time was really needed
    real32 render_time__seconds;                       // See
    real32 total_frame_time_elapsed__seconds;          // After the sleep or vsync block

    real64 physics_simluation_elapsed_time__seconds;
};

#ifdef __WIN32__
uint64 global_last_cycle_count;
uint64 global_cycles_elapsed_before_render;
uint64 global_cycles_elapsed_after_render;
uint64 global_total_cycles_elapsed;
#endif

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

    if (TTF_Init() == -1)
    {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        return -1;
    }

    global_window = SDL_CreateWindow("SDL Starter",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     LOGICAL_WIDTH,
                                     LOGICAL_HEIGHT,
                                     SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
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

    // const char* platform = SDL_GetPlatform();
    // std::cout << "Platform " << platform << std::endl;

#ifdef __WINDOWS__
    timeBeginPeriod(1);
#endif

// Set macOS-specific hint
#ifdef __APPLE__
    SDL_SetHint(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, "1");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");
#endif

    global_font = TTF_OpenFont("fonts/Roboto/Roboto-Medium.ttf", 256);
    if (global_font == nullptr)
    {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return -1;
    }

    global_debug_font = TTF_OpenFont("fonts/Roboto/Roboto-Medium.ttf", 16);
    if (global_debug_font == nullptr)
    {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return -1;
    }

    SDL_Event event;
    Input input = {};

    SDL_SetEventFilter(filterEvent, &input);

    GLOBAL_PERFORMANCE_FREQUENCY = SDL_GetPerformanceFrequency();

    Master_Timer master_timer = {};
    master_timer.COUNTER_FREQUENCY = GLOBAL_PERFORMANCE_FREQUENCY;
    master_timer.last_frame_counter = SDL_GetPerformanceCounter();

    global_debug_counter = 0;
    global_tick_counter_before = SDL_GetPerformanceCounter();

    global_counter_last_frame = SDL_GetPerformanceCounter();

    real32 accumulator_s = 0.0f;

    global_paused = 0;
    global_display_debug_info = 0;

    State starting_state = {};
    starting_state.pos_y = -30;
    starting_state.direction = DIRECTION_NORTH;

    State previous_state = starting_state;
    State current_state = starting_state;

#ifdef __WIN32__
    // This looks like a function call but it's actually an intrinsic that
    // runs the actual assembly instruction directly
    global_last_cycle_count = __rdtsc();
#endif

#ifdef __WIN32__
    char mega_cycles_text[100] = "";
    char actual_mega_cycles_text[100] = "";
    char render_mega_cycles_text[100] = "";
#endif

    char fps_text[100] = "";
    char ms_per_frame_text[100] = "";
    char work_ms_per_frame_text[100] = "";
    char render_ms_per_frame_text[100] = "";

    bool32 vsync_enabled = 1;

    SDL_RenderSetVSync(global_renderer, vsync_enabled);

    while (global_running)
    {
#ifdef __WIN32__
        uint64 global_cycle_count_now = __rdtsc();
        global_cycles_elapsed_before_render = global_cycle_count_now - global_last_cycle_count;
#endif

        Uint64 counter_now = SDL_GetPerformanceCounter();

        master_timer.frame_time_elapsed_before_render__seconds =
            ((real32)(counter_now - master_timer.last_frame_counter) / (real32)master_timer.COUNTER_FREQUENCY);

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
                // Prevent "spiraling" (excessive frame accumulation) in case of a big lag spike.
                frame_time_s = 0.25f;
            }

            accumulator_s += frame_time_s;

            while (accumulator_s >= SIMULATION_DELTA_TIME_S)
            {  // Simulation 'consumes' whatever time is given to it based on the render rate
                previous_state = current_state;
                simulate(&current_state,
                         &input,
                         master_timer.physics_simluation_elapsed_time__seconds,
                         SIMULATION_DELTA_TIME_S);
                master_timer.physics_simluation_elapsed_time__seconds += SIMULATION_DELTA_TIME_S;
                accumulator_s -= SIMULATION_DELTA_TIME_S;
            }
        }

        real32 alpha = accumulator_s / SIMULATION_DELTA_TIME_S;
        // Interpolate between the current state and previous state
        // NOTE: the render always lags by about a frame
        State state = current_state * alpha + previous_state * (1.0f - alpha);

        // TODO: WORK
        global_debug_counter += master_timer.total_frame_time_elapsed__seconds;
        // Tick every second
        if (global_debug_counter >= 0.2)
        {
            global_debug_counter = 0;
        }

        {  // Render
            // Clear the screen
            SDL_SetRenderDrawColor(global_renderer, 0, 0, 0, 255);  // Black background
            SDL_RenderClear(global_renderer);

            render(&state);

#if 0
        SDL_Color text_color = {255, 255, 255};  // White color
        render_centered_text_with_scaling("Snake Game",
                                          LOGICAL_WIDTH / 2, 50,
                                          LOGICAL_WIDTH * 0.25f,
                                          text_color);
#endif

#if 0
        {  // Eat CPU time
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

#if 0
        // Eat CPU time to test debug stuff
        uint64 target_cycles = (uint64)(0.01 * 1000.0f * 1000.0f * 1000.0f); // Adjust this to simulate the desired load
        uint64 start_cycles = __rdtsc();

        while (true) {
            // Perform some dummy operations to keep the CPU busy
            volatile uint32 dummy = 0;
            for (uint32 i = 0; i < 1000; ++i) {
                dummy += i;
            }

            // Check the current cycle count
            uint64 current_cycles = __rdtsc();
            if ((current_cycles - start_cycles) >= target_cycles) {
                break;
            }
        }
#endif
        }
#endif

            if (global_display_debug_info)  // Render Debug Info
            {
                // Disable logical size scaling temporarily
                SDL_RenderSetLogicalSize(global_renderer, 0, 0);

                SDL_Color debug_text_color = {255, 255, 255};  // White color
                real32 y_offset = 0;
                int32 font_height = TTF_FontHeight(global_debug_font);
                real32 padding = 5.0f;
                real32 vertical_offset = font_height + padding;

#ifdef __WIN32__
                {  // Mega Cycles Per Frame
                    real64 mega_cycles_per_frame = global_total_cycles_elapsed / (1000.0f * 1000.0f);

                    if (global_debug_counter == 0)
                    {
                        snprintf(mega_cycles_text,
                                 sizeof(mega_cycles_text),
                                 "Mega cycles/Frame: %.02f",
                                 mega_cycles_per_frame);
                    }

                    render_text_no_scaling(mega_cycles_text,
                                           (int32)(LOGICAL_WIDTH * 0.01f),
                                           (int32)y_offset + (int32)(LOGICAL_HEIGHT * 0.01f),
                                           debug_text_color);
                    y_offset += vertical_offset;
                }

                {  // Work Cycles Per Frame
                    real64 mega_cycles_for_actual_work = global_cycles_elapsed_before_render / (1000.0f * 1000.0f);

                    if (global_debug_counter == 0)
                    {
                        snprintf(actual_mega_cycles_text,
                                 sizeof(actual_mega_cycles_text),
                                 "Work mega cycles/Frame: %.02f",
                                 mega_cycles_for_actual_work);
                    }

                    render_text_no_scaling(actual_mega_cycles_text,
                                           (int32)(LOGICAL_WIDTH * 0.01f),
                                           (int32)y_offset + (int32)(LOGICAL_HEIGHT * 0.01f),
                                           debug_text_color);
                    y_offset += vertical_offset;
                }

                {  // Render Cycles Per Frame
                    real64 mega_cycles_for_render = global_cycles_elapsed_after_render / (1000.0f * 1000.0f);

                    if (global_debug_counter == 0)
                    {
                        snprintf(render_mega_cycles_text,
                                 sizeof(render_mega_cycles_text),
                                 "Render mega cycles/Frame: %.02f",
                                 mega_cycles_for_render);
                    }

                    render_text_no_scaling(render_mega_cycles_text,
                                           (int32)(LOGICAL_WIDTH * 0.01f),
                                           (int32)y_offset + (int32)(LOGICAL_HEIGHT * 0.01f),
                                           debug_text_color);
                    y_offset += vertical_offset;
                }
#endif

                {  // FPS
                    real32 fps = 1.0f / master_timer.total_frame_time_elapsed__seconds;

                    if (global_debug_counter == 0)
                    {
                        snprintf(fps_text, sizeof(fps_text), "FPS: %.02f", fps);
                    }

                    render_text_no_scaling(fps_text,
                                           (int32)(LOGICAL_WIDTH * 0.01f),
                                           (int32)y_offset + (int32)(LOGICAL_HEIGHT * 0.01f),
                                           debug_text_color);
                    y_offset += vertical_offset;
                }

                real32 ms_per_frame = master_timer.total_frame_time_elapsed__seconds * 1000.0f;

                {  // Total Frame Time (MS)

                    if (global_debug_counter == 0)
                    {
                        snprintf(ms_per_frame_text,
                                 sizeof(ms_per_frame_text),
                                 "Ms/frame: %.04f (Target: %.04f)",
                                 ms_per_frame,
                                 TARGET_TIME_PER_FRAME_MS);
                    }

                    render_text_no_scaling(ms_per_frame_text,
                                           (int32)(LOGICAL_WIDTH * 0.01f),
                                           (int32)y_offset + (int32)(LOGICAL_HEIGHT * 0.01f),
                                           debug_text_color);
                    y_offset += vertical_offset;
                }

                {  // Work Frame Time (MS)
                    real32 work_ms_per_frame = master_timer.frame_time_elapsed_before_render__seconds * 1000.0f;

                    SDL_Color work_ms_per_frame_text_color;

                    if (work_ms_per_frame < TARGET_TIME_PER_FRAME_MS)
                    {
                        work_ms_per_frame_text_color = debug_text_color;
                    }
                    else
                    {
                        work_ms_per_frame_text_color.r = 185;
                        work_ms_per_frame_text_color.g = 80;
                        work_ms_per_frame_text_color.b = 75;
                        work_ms_per_frame_text_color.a = 255;  // Set alpha to fully opaque (or other value as needed)
                    }

                    if (global_debug_counter == 0)
                    {
                        snprintf(work_ms_per_frame_text,
                                 sizeof(work_ms_per_frame_text),
                                 "Work ms: %.04f, (%.1f%%)",
                                 work_ms_per_frame,
                                 (work_ms_per_frame / ms_per_frame) * 100);
                    }

                    render_text_no_scaling(work_ms_per_frame_text,
                                           (int32)(LOGICAL_WIDTH * 0.01f),
                                           (int32)y_offset + (int32)(LOGICAL_HEIGHT * 0.01f),
                                           work_ms_per_frame_text_color);
                    y_offset += vertical_offset;
                }

                {  // Render Frame Time (MS)
                    real32 render_ms_per_frame = master_timer.render_time__seconds * 1000.0f;

                    if (global_debug_counter == 0)
                    {
                        snprintf(render_ms_per_frame_text,
                                 sizeof(render_ms_per_frame_text),
                                 "Render ms: %.04f, (%.1f%%)",
                                 render_ms_per_frame,
                                 (render_ms_per_frame / ms_per_frame) * 100);
                    }

                    render_text_no_scaling(render_ms_per_frame_text,
                                           (int32)(LOGICAL_WIDTH * 0.01f),
                                           (int32)y_offset + (int32)(LOGICAL_HEIGHT * 0.01f),
                                           debug_text_color);
                    y_offset += vertical_offset;
                }

                // Re-enable logical size scaling for other elements
                SDL_RenderSetLogicalSize(global_renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);
            }

            if (global_debug_counter == 0)  // FPS Timer in Window Name
            {
                real32 fps = 1.0f / master_timer.total_frame_time_elapsed__seconds;

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

            // Present the rendered content
            SDL_RenderPresent(global_renderer);
            SDL_SetRenderTarget(global_renderer, NULL);
        }

#ifdef __WIN32__
        uint64 global_cycle_count_after_render = __rdtsc();
        global_cycles_elapsed_after_render = global_cycle_count_after_render - global_cycle_count_now;
#endif

        Uint64 counter_after_render = SDL_GetPerformanceCounter();
        master_timer.render_time__seconds =
            ((real32)(counter_after_render - counter_now) / (real32)master_timer.COUNTER_FREQUENCY);

        if (!vsync_enabled)
        {
            real32 sleep_time_s = TARGET_TIME_PER_FRAME_S - master_timer.frame_time_elapsed_before_render__seconds;

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
        master_timer.total_frame_time_elapsed__seconds =
            ((real32)(counter_after_sleep - master_timer.last_frame_counter) /
             (real32)master_timer.COUNTER_FREQUENCY);

        // Next iteration
        master_timer.last_frame_counter = counter_after_sleep;

#ifdef __WIN32__
        global_last_cycle_count = global_end_cycle_count_after_delay;
#endif
    }

    TTF_CloseFont(global_font);
    TTF_CloseFont(global_debug_font);
    SDL_DestroyRenderer(global_renderer);
    SDL_DestroyWindow(global_window);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}