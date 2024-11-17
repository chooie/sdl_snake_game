// clang-format off
#include "common.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#ifdef __WINDOWS__
#include <windows.h>
#include <mmsystem.h>
#endif
// clang-format on

bool32 TEXT_DEBUGGING_ENABLED = 1;
bool32 VSYNC_ENABLED = 1;
real32 TARGET_SCREEN_FPS = 58.9f;

int32 LOGICAL_WIDTH = 1280;
int32 LOGICAL_HEIGHT = 720;
real32 ABSOLUTE_ASPECT_RATIO = (real32)LOGICAL_WIDTH / (real32)LOGICAL_HEIGHT;
/*
Here are a few integer grid sizes that would fit a 16:9 aspect ratio perfectly:

16x9 Grid - Each cell would be 80x80 pixels.
32x18 Grid - Each cell would be 40x40 pixels.
64x36 Grid - Each cell would be 20x20 pixels.
80x45 Grid - Each cell would be 16x16 pixels.
160x90 Grid - Each cell would be 8x8 pixels.
320x180 Grid - Each cell would be 4x4 pixels.
640x360 Grid - Each cell would be 2x2 pixels.
*/
uint32 GRID_BLOCK_SIZE = 20;
uint32 X_GRIDS = LOGICAL_WIDTH / GRID_BLOCK_SIZE;  // Should exactly divide into logical width
uint32 Y_GRIDS = (int32)(X_GRIDS / ABSOLUTE_ASPECT_RATIO);

int32 window_width = LOGICAL_WIDTH;
int32 window_height = LOGICAL_HEIGHT;

real32 SIMULATION_FPS = 100;
real32 SIMULATION_DELTA_TIME_S = 1.f / SIMULATION_FPS;

real32 TARGET_TIME_PER_FRAME_S = 1.f / (real32)TARGET_SCREEN_FPS;
real32 TARGET_TIME_PER_FRAME_MS = TARGET_TIME_PER_FRAME_S * 1000.0f;

int32 global_running = 1;
SDL_Window* global_window;
SDL_Renderer* global_renderer;

real32 global_text_dpi_scale_factor;

bool32 global_display_debug_info;
bool32 global_paused;

real32 global_debug_counter;

struct Master_Timer
{
    // High-res timer stuff
    Uint64 last_frame_counter;
    Uint64 COUNTER_FREQUENCY;

    real32 time_elapsed_for_work__seconds;            // How much time was needed for simulation stuff
    real32 time_elapsed_for_writing_buffer__seconds;  // How much time was needed for writing to render buffer?
    real32 time_elapsed_for_render__seconds;          // How much time was needed for rendering?
    real32 time_elapsed_for_sleep__seconds;           // How much time was needed for sleeping?
    real32 total_frame_time_elapsed__seconds;         // How long did the whole dang frame take?
    real64 physics_simulation_elapsed_time__seconds;  // This is the main counter for time. Everything will rely on what
                                                      // the physics sees
};

void set_dpi()
{
    real32 dpi;
    if (SDL_GetDisplayDPI(0, &dpi, NULL, NULL) == 0) {
        // Successfully retrieved DPI
    } else {
        // Handle error
    }

    real32 base_DPI = 72.0f;
    global_text_dpi_scale_factor = dpi / base_DPI;
}

#ifdef __WIN32__
uint64 global_last_cycle_count;
uint64 global_cycles_elapsed_before_render;
uint64 global_cycles_elapsed_after_render;
uint64 global_total_cycles_elapsed;
#endif

// clang-format off
#include "input.cpp"
#include "game.cpp"
#include "render.cpp"
#include "audio.cpp"
// clang-format on

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

void reset_state(State* state)
{
    head = 0;
    tail = 0;

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

int32 main(int32 argc, char* argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    if (TTF_Init() == -1)
    {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        return -1;
    }

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    Audio_Context audio_ctx;
    if (!audio_init(&audio_ctx)) {
        fprintf(stderr, "Failed to initialize audio.\n");
        SDL_Quit();
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

    set_dpi();

    SDL_Event event;
    Input input = {};

    SDL_SetEventFilter(filterEvent, &input);

    Master_Timer master_timer = {};
    master_timer.COUNTER_FREQUENCY = SDL_GetPerformanceFrequency();
    master_timer.last_frame_counter = SDL_GetPerformanceCounter();

    global_debug_counter = 0;

    real32 accumulator_s = 0.0f;

    global_paused = 1;
    global_display_debug_info = 0;

    State starting_state = {};
    reset_state(&starting_state);

    State previous_state = starting_state;
    State current_state = starting_state;

    bool32 is_first_run = 1;

#ifdef __WIN32__
    // This looks like a function call but it's actually an intrinsic that
    // runs the actual assembly instruction directly
    global_last_cycle_count = __rdtsc();
#endif

#define DEBUG_TEXT_STRING_LENGTH 100

#ifdef __WIN32__
    char mega_cycles_text[DEBUG_TEXT_STRING_LENGTH] = "";
    char actual_mega_cycles_text[DEBUG_TEXT_STRING_LENGTH] = "";
    char render_mega_cycles_text[DEBUG_TEXT_STRING_LENGTH] = "";
#endif

    SDL_Color white_text_color = { 255, 255, 255, 255 }; // White color
    real32 font_size = 16.0f;

    Drawn_Text_Static score_drawn_text_static = {};
    {
        score_drawn_text_static.text_string = "SCORE";
    }
    score_drawn_text_static.font_size = font_size;
    score_drawn_text_static.color = white_text_color;

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

#define DYNAMIC_SCORE_LENGTH 5

    char dynamic_score_text[DYNAMIC_SCORE_LENGTH]; // Make sure the buffer is large enough
    Drawn_Text_Int32 score_drawn_text_dynamic = {};
    score_drawn_text_dynamic.original_value = 0;
    score_drawn_text_dynamic.text_string = dynamic_score_text;
    score_drawn_text_dynamic.font_size = font_size;
    score_drawn_text_dynamic.color = white_text_color;


    real32 debug_x_start_offset = (int32)(LOGICAL_WIDTH * 0.01f);
    real32 debug_y_start_offset = (int32)(LOGICAL_HEIGHT * 0.01f);
    real32 debug_padding = 5.0f;

    // Get font height for offset
    real32 font_height;
    {
        real32 font_pt_size = get_font_pt_size(font_size);
        TTF_Font* font = get_font(font_pt_size);
        font_height = TTF_FontHeight(font) / global_text_dpi_scale_factor;
    }
    real32 vertical_offset = font_height + debug_padding;
    real32 y_offset = 0;

    char fps_text[DEBUG_TEXT_STRING_LENGTH] = "";
    Drawn_Text fps_drawn_text = {};
    fps_drawn_text.original_value = 0.f;
    fps_drawn_text.text_string = fps_text;
    fps_drawn_text.font_size = font_size;
    fps_drawn_text.color = white_text_color;
    fps_drawn_text.text_rect.x = debug_x_start_offset;
    fps_drawn_text.text_rect.y = debug_x_start_offset + y_offset;
    y_offset += vertical_offset;

    char ms_per_frame_text[DEBUG_TEXT_STRING_LENGTH] = "";
    Drawn_Text ms_per_frame_drawn_text = {};
    ms_per_frame_drawn_text.original_value = 0.f;
    ms_per_frame_drawn_text.text_string = ms_per_frame_text;
    ms_per_frame_drawn_text.font_size = font_size;
    ms_per_frame_drawn_text.color = white_text_color;
    ms_per_frame_drawn_text.text_rect.x = debug_x_start_offset;
    ms_per_frame_drawn_text.text_rect.y = debug_x_start_offset + y_offset;
    y_offset += vertical_offset;

    char work_ms_per_frame_text[DEBUG_TEXT_STRING_LENGTH] = "";
    Drawn_Text work_ms_per_frame_drawn_text = {};
    work_ms_per_frame_drawn_text.original_value = 0.f;
    work_ms_per_frame_drawn_text.text_string = work_ms_per_frame_text;
    work_ms_per_frame_drawn_text.font_size = font_size;
    work_ms_per_frame_drawn_text.color = white_text_color;
    work_ms_per_frame_drawn_text.text_rect.x = debug_x_start_offset;
    work_ms_per_frame_drawn_text.text_rect.y = debug_x_start_offset + y_offset;
    y_offset += vertical_offset;

    char writing_buffer_ms_per_frame_text[DEBUG_TEXT_STRING_LENGTH] = "";
    Drawn_Text writing_buffer_ms_per_frame_drawn_text = {};
    writing_buffer_ms_per_frame_drawn_text.original_value = 0.f;
    writing_buffer_ms_per_frame_drawn_text.text_string = writing_buffer_ms_per_frame_text;
    writing_buffer_ms_per_frame_drawn_text.font_size = font_size;
    writing_buffer_ms_per_frame_drawn_text.color = white_text_color;
    writing_buffer_ms_per_frame_drawn_text.text_rect.x = debug_x_start_offset;
    writing_buffer_ms_per_frame_drawn_text.text_rect.y = debug_x_start_offset + y_offset;
    y_offset += vertical_offset;

    char render_ms_per_frame_text[DEBUG_TEXT_STRING_LENGTH] = "";
    Drawn_Text render_ms_per_frame_drawn_text = {};
    render_ms_per_frame_drawn_text.original_value = 0.f;
    render_ms_per_frame_drawn_text.text_string = render_ms_per_frame_text;
    render_ms_per_frame_drawn_text.font_size = font_size;
    render_ms_per_frame_drawn_text.color = white_text_color;
    render_ms_per_frame_drawn_text.text_rect.x = debug_x_start_offset;
    render_ms_per_frame_drawn_text.text_rect.y = debug_x_start_offset + y_offset;
    y_offset += vertical_offset;

    char sleep_ms_per_frame_text[DEBUG_TEXT_STRING_LENGTH] = "";
    Drawn_Text sleep_ms_per_frame_drawn_text = {};
    sleep_ms_per_frame_drawn_text.original_value = 0.f;
    sleep_ms_per_frame_drawn_text.text_string = sleep_ms_per_frame_text;
    sleep_ms_per_frame_drawn_text.font_size = font_size;
    sleep_ms_per_frame_drawn_text.color = white_text_color;
    sleep_ms_per_frame_drawn_text.text_rect.x = debug_x_start_offset;
    sleep_ms_per_frame_drawn_text.text_rect.y = debug_x_start_offset + y_offset;
    y_offset += vertical_offset;

    SDL_RenderSetVSync(global_renderer, VSYNC_ENABLED);

    while (global_running)
    {
        if (is_first_run) {
            play_music(&audio_ctx);
            set_music_volume(10.f);
        }

        real32 LAST_frame_time_elapsed_for_work__seconds = master_timer.time_elapsed_for_work__seconds;
        real32 LAST_frame_time_elapsed_for_writing_buffer__seconds = master_timer.time_elapsed_for_writing_buffer__seconds;
        real32 LAST_frame_time_elapsed_for_render__seconds = master_timer.time_elapsed_for_render__seconds;
        real32 LAST_frame_time_elapsed_for_sleep__seconds = master_timer.time_elapsed_for_sleep__seconds;
        real32 total_LAST_frame_time_elapsed__seconds = master_timer.total_frame_time_elapsed__seconds;

#ifdef __WIN32__
        uint64 global_cycle_count_now = __rdtsc();
        global_cycles_elapsed_before_render = global_cycle_count_now - global_last_cycle_count;
#endif

        Uint64 counter_now = SDL_GetPerformanceCounter();

        handle_input(&event, &input);

        if (!global_paused) {

            if (pressed_local(BUTTON_W) || pressed_local(BUTTON_UP))
            {
                add_input(DIRECTION_NORTH);
            }

            if (pressed_local(BUTTON_A) || pressed_local(BUTTON_LEFT))
            {
                add_input(DIRECTION_WEST);
            }

            if (pressed_local(BUTTON_S) || pressed_local(BUTTON_DOWN))
            {
                add_input(DIRECTION_SOUTH);
            }

            if (pressed_local(BUTTON_D) || pressed_local(BUTTON_RIGHT))
            {
                add_input(DIRECTION_EAST);
            }
        }

        State state;

        { // Handle Some Input in here?
            if (state.game_over && pressed_local(BUTTON_ENTER))
            {
                reset_state(&previous_state);
                reset_state(&current_state);
                reset_state(&state);
            }
        }

        { // Simulation work
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
                             &audio_ctx,
                             master_timer.physics_simulation_elapsed_time__seconds,
                             SIMULATION_DELTA_TIME_S);
                    master_timer.physics_simulation_elapsed_time__seconds += SIMULATION_DELTA_TIME_S;
                    accumulator_s -= SIMULATION_DELTA_TIME_S;
                }
            }

            real32 alpha = accumulator_s / SIMULATION_DELTA_TIME_S;
            // Interpolate between the current state and previous state
            // NOTE: the render always lags by about a frame

            // NOTE: I've commented this out because I don't think we need linear interpolation for new grid-based approach?
            // state = current_state * alpha + previous_state * (1.0f - alpha);
            state = current_state;

            {  // Tick debug text counter
                global_debug_counter += master_timer.total_frame_time_elapsed__seconds;
                // Tick every second
                if (global_debug_counter >= 1.0f)
                {
                    global_debug_counter = 0;
                }
            }

#if 0 // Eat CPU time to test debug stuff
            {  // Eat CPU time
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
            }
#endif
        }

        Uint64 counter_after_work = SDL_GetPerformanceCounter();
        master_timer.time_elapsed_for_work__seconds =
            ((real32)(counter_after_work - counter_now) / (real32)master_timer.COUNTER_FREQUENCY);

        { // Write to render buffer
            // Clear the screen
            SDL_SetRenderDrawColor(global_renderer, 0, 0, 0, 255);  // Black background
            SDL_RenderClear(global_renderer);

#if 1
            render(&state);
#if 1
            { // Render score
                int32 pt_size = (int32)(0.5f + font_size * global_text_dpi_scale_factor);
                TTF_Font* font = get_font(pt_size);

                
                int32 OFFSET = 40;
                score_drawn_text_static.text_rect.x = LOGICAL_WIDTH - score_drawn_text_static.text_rect.w - OFFSET;
                score_drawn_text_static.text_rect.y = 0;
                draw_text_static(&score_drawn_text_static);

                // ==========================

                if (is_first_run || state.next_snake_part_index != score_drawn_text_dynamic.original_value)
                {
                    snprintf(dynamic_score_text, DYNAMIC_SCORE_LENGTH, "%d", state.next_snake_part_index);
                }

                score_drawn_text_dynamic.text_rect.x = score_drawn_text_static.text_rect.x + 5 + score_drawn_text_static.text_rect.w;
                score_drawn_text_dynamic.text_rect.y = 0; 
                draw_text_int32(&score_drawn_text_dynamic, is_first_run, state.next_snake_part_index);
            }

            { // Render Game Over
                if (state.game_over)
                {
                    draw_text_static(&game_over_drawn_text_static);
                    game_over_drawn_text_static.text_rect.x = LOGICAL_WIDTH / 2;
                    game_over_drawn_text_static.text_rect.y = LOGICAL_HEIGHT / 2;

                    game_over_drawn_text_static.text_rect.x -= game_over_drawn_text_static.text_rect.w / 2;
                    game_over_drawn_text_static.text_rect.y -= game_over_drawn_text_static.text_rect.h / 2;

                    draw_text_static(&restart_drawn_text_static);
                    restart_drawn_text_static.text_rect.x = LOGICAL_WIDTH / 2;
                    restart_drawn_text_static.text_rect.y = LOGICAL_HEIGHT / 2;
                    restart_drawn_text_static.text_rect.x -= restart_drawn_text_static.text_rect.w / 2;
                    restart_drawn_text_static.text_rect.y -= restart_drawn_text_static.text_rect.h / 2;

                    // Place text under other text
                    restart_drawn_text_static.text_rect.y += game_over_drawn_text_static.text_rect.h;
                }
            }
#endif
#endif
            if (TEXT_DEBUGGING_ENABLED) // Displays Debug info in the console
            {
                { // FPS
                    real32 fps = 1.0f / total_LAST_frame_time_elapsed__seconds;
                    if (global_debug_counter == 0)
                    {
                        printf("FPS: %.1f, ", fps);
                    }
                }

                real32 ms_per_frame = total_LAST_frame_time_elapsed__seconds * 1000.0f;
                { // Total Frame Time (MS)
                    if (global_debug_counter == 0)
                    {
                        printf("Ms/frame: %.04f (Target: %.04f), ",
                               ms_per_frame,
                               TARGET_TIME_PER_FRAME_MS);
                    }
                }

                { // Work Frame Time (MS)
                    real32 work_ms_per_frame = LAST_frame_time_elapsed_for_work__seconds * 1000.0f;

                    if (global_debug_counter == 0)
                    {
                        printf("Work ms: %.04f, (%.1f%%), ",
                               work_ms_per_frame,
                               (work_ms_per_frame / ms_per_frame) * 100);
                    }
                }

                { // Buffer Ms
                    real32 writing_buffer_ms_per_frame = LAST_frame_time_elapsed_for_writing_buffer__seconds * 1000.0f;
                    if (global_debug_counter == 0)
                    {
                        printf("Buffer ms: %.04f, (%.1f%%), ",
                               writing_buffer_ms_per_frame,
                               (writing_buffer_ms_per_frame / ms_per_frame) * 100);
                    }
                }

                { // Render Frame Time (MS)
                    real32 render_ms_per_frame = LAST_frame_time_elapsed_for_render__seconds * 1000.0f;

                    if (global_debug_counter == 0)
                    {
                        printf("Render ms: %.04f, (%.1f%%), ",
                               render_ms_per_frame,
                               (render_ms_per_frame / ms_per_frame) * 100);
                    }
                }

                { // Sleep Frame Time (MS)
                    real32 sleep_ms_per_frame = LAST_frame_time_elapsed_for_sleep__seconds * 1000.0f;

                    if (global_debug_counter == 0)
                    {
                        printf("Sleep ms: %.04f, (%.1f%%)",
                               sleep_ms_per_frame,
                               (sleep_ms_per_frame / ms_per_frame) * 100);
                    }
                }

                if (global_debug_counter == 0) {
                    printf("\n");
                }

#if 0
                if (global_debug_counter == 0)
                {
                    printf("X Grids: %d, Y Grids: %d, X: %d, Y: %d\n", X_GRIDS, Y_GRIDS, state.blip_pos_x, state.blip_pos_y);
                }
#endif
            }

#if 1 // Render Debug Info
            if (global_display_debug_info)  // Render Debug Info
            {
                // TODO: remove this when changing the win32 stuff
                SDL_Color debug_text_color = {255, 255, 255, 255};  // White color
                real32 font_size = 16.0f;
                int32 pt_size = (int32)(0.5f + font_size * global_text_dpi_scale_factor);
                real32 y_offset = 0;
                real32 padding = 5.0f;
                real32 font_height;
                real32 vertical_offset;

#ifdef __WIN32__
                { // Mega Cycles Per Frame
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

                { // Work Cycles Per Frame
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

                { // Render Cycles Per Frame
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

                { // FPS
                    real32 fps = 1.0f / total_LAST_frame_time_elapsed__seconds;

                    if (global_debug_counter == 0)
                    {
                        snprintf(fps_text, sizeof(fps_text), "FPS: %.02f", fps);
                    }

                    draw_text_real32(&fps_drawn_text, is_first_run, fps);
                }

                real32 ms_per_frame = total_LAST_frame_time_elapsed__seconds * 1000.0f;

                { // Total Frame Time (MS)
                    if (global_debug_counter == 0)
                    {
                        snprintf(ms_per_frame_text,
                                 sizeof(ms_per_frame_text),
                                 "Ms/frame: %.04f (Target: %.04f)",
                                 ms_per_frame,
                                 TARGET_TIME_PER_FRAME_MS);
                    }

                    draw_text_real32(&ms_per_frame_drawn_text, is_first_run, ms_per_frame);
                }

                { // Work Frame Time (MS)
                    real32 work_ms_per_frame = LAST_frame_time_elapsed_for_work__seconds * 1000.0f;

                    if (global_debug_counter == 0)
                    {
                        snprintf(work_ms_per_frame_text,
                                 sizeof(work_ms_per_frame_text),
                                 "Work ms: %.04f, (%.1f%%)",
                                 work_ms_per_frame,
                                 (work_ms_per_frame / ms_per_frame) * 100);
                    }

                    draw_text_real32(&work_ms_per_frame_drawn_text, is_first_run, work_ms_per_frame);
                }

                { // Buffer Writing Time (MS)
                    real32 writing_buffer_ms_per_frame = LAST_frame_time_elapsed_for_writing_buffer__seconds * 1000.0f;

                    if (global_debug_counter == 0)
                    {
                        snprintf(writing_buffer_ms_per_frame_text,
                                 sizeof(writing_buffer_ms_per_frame_text),
                                 "Buffer ms: %.04f, (%.1f%%)",
                                 writing_buffer_ms_per_frame,
                                 (writing_buffer_ms_per_frame / ms_per_frame) * 100);
                    }

                    draw_text_real32(&writing_buffer_ms_per_frame_drawn_text, is_first_run, writing_buffer_ms_per_frame);
                }

                { // Render Frame Time (MS)
                    real32 render_ms_per_frame = LAST_frame_time_elapsed_for_render__seconds * 1000.0f;

                    if (global_debug_counter == 0)
                    {
                        snprintf(render_ms_per_frame_text,
                                 sizeof(render_ms_per_frame_text),
                                 "Render ms: %.04f, (%.1f%%)",
                                 render_ms_per_frame,
                                 (render_ms_per_frame / ms_per_frame) * 100);
                    }

                   draw_text_real32(&render_ms_per_frame_drawn_text, is_first_run, render_ms_per_frame);
                }

                { // Sleep Frame Time (MS)
                    real32 sleep_ms_per_frame = LAST_frame_time_elapsed_for_sleep__seconds * 1000.0f;

                    if (global_debug_counter == 0)
                    {
                        snprintf(sleep_ms_per_frame_text,
                                 sizeof(sleep_ms_per_frame_text),
                                 "Sleep ms: %.04f, (%.1f%%)",
                                 sleep_ms_per_frame,
                                 (sleep_ms_per_frame / ms_per_frame) * 100);
                    }

                    draw_text_real32(&sleep_ms_per_frame_drawn_text, is_first_run, sleep_ms_per_frame);
                }
            }
#endif

#if 1
            if (global_debug_counter == 0)  // FPS Timer in Window Name
            {
                real32 fps = 1.0f / master_timer.total_frame_time_elapsed__seconds;

                char fps_str[30];  // Allocate enough space for the string
                snprintf(fps_str, 30, "%.2f", fps);

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
        }

        Uint64 counter_after_writing_buffer = SDL_GetPerformanceCounter();
        master_timer.time_elapsed_for_writing_buffer__seconds =
            ((real32)(counter_after_writing_buffer - counter_after_work) / (real32)master_timer.COUNTER_FREQUENCY);

#if 1 // Render Present
        { // Present the rendered content (Will block for vsync)
            SDL_RenderPresent(global_renderer);
        }
#endif

#ifdef __WIN32__
        uint64 global_cycle_count_after_render = __rdtsc();
        global_cycles_elapsed_after_render = global_cycle_count_after_render - global_cycle_count_now;
#endif

        Uint64 counter_after_render = SDL_GetPerformanceCounter();
        master_timer.time_elapsed_for_render__seconds =
            ((real32)(counter_after_render - counter_after_writing_buffer) / (real32)master_timer.COUNTER_FREQUENCY);
#if 1
        { // Sleep
            uint64 MICRO = 1000000;
            const Uint64 TARGET_FRAME_DURATION = MICRO / TARGET_SCREEN_FPS;  // In microseconds
            int64 target_duration_ticks =
                (TARGET_FRAME_DURATION * master_timer.COUNTER_FREQUENCY) / MICRO;  // Convert to ticks
            Uint64 elapsed_ticks = counter_after_render - counter_now;

            if (elapsed_ticks < target_duration_ticks)
            {
                Uint64 remaining_ticks = target_duration_ticks - elapsed_ticks;
                Uint64 remaining_microseconds = MICRO * (remaining_ticks / master_timer.COUNTER_FREQUENCY);

                // Use SDL_Delay for most of the remaining time if it's large enough
                if (remaining_microseconds > 1000)
                {
                    SDL_Delay(remaining_microseconds / 1000);  // Delay in milliseconds
                }

                // Spin-wait for the remaining time (microsecond precision)
                while (SDL_GetPerformanceCounter() - counter_now < target_duration_ticks)
                {
                    // Busy-wait for precise timing
                }
            }
        }
#endif

#ifdef __WIN32__
        uint64 global_end_cycle_count_after_delay = __rdtsc();
        global_total_cycles_elapsed = global_end_cycle_count_after_delay - global_last_cycle_count;
#endif

        Uint64 counter_after_sleep = SDL_GetPerformanceCounter();
        master_timer.time_elapsed_for_sleep__seconds =
            ((real32)(counter_after_sleep - counter_after_render) / (real32)master_timer.COUNTER_FREQUENCY);
        master_timer.total_frame_time_elapsed__seconds =
            ((real32)(counter_after_sleep - counter_now) / (real32)master_timer.COUNTER_FREQUENCY);

        // Next iteration
        master_timer.last_frame_counter = counter_after_sleep;

#ifdef __WIN32__
        global_last_cycle_count = global_end_cycle_count_after_delay;
#endif

        is_first_run = 0;
    }

    cleanup_fonts();
    SDL_DestroyRenderer(global_renderer);
    SDL_DestroyWindow(global_window);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}