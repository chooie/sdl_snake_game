real32 ACCELERATION_POWER = 2000;
real32 friction = 10.0f;

bool32 is_first_run = 1;

real32 global_velocity_x = 0;
real32 global_velocity_y = 0;

real32 global_square_x;
real32 global_square_y;

real32 render_scale = 0.01f;

struct Color_RGBA
{
    int8 red;
    int8 green;
    int8 blue;
    int8 alpha;
};

/*
Coords are in -100 to 100 space
*/
void draw_square(real32 x, real32 y, real32 size, Color_RGBA color)
{
    real32 normalized_x = x;
    real32 normalized_y = y;
}

void update(Input* input, real32 dt_s)
{
    real32 acceleration_x = 0;
    real32 acceleration_y = 0;

    if (is_down(BUTTON_W))
    {
        acceleration_y += -ACCELERATION_POWER;
        printf("Holding down W...\n");
    }

    if (is_down(BUTTON_A))
    {
        acceleration_x += -ACCELERATION_POWER;
        printf("Holding down A...\n");
    }

    if (is_down(BUTTON_S))
    {
        acceleration_y += ACCELERATION_POWER;
        printf("Holding down S...\n");
    }

    if (is_down(BUTTON_D))
    {
        acceleration_x += ACCELERATION_POWER;
        printf("Holding down D...\n");
    }

    // Slow the player down through friction
    acceleration_x -= global_velocity_x * friction;
    acceleration_y -= global_velocity_y * friction;

    if (is_first_run)
    {
        is_first_run = 0;

        // Coordinates -100 to +100 on x and y

        global_square_x = 0;
        global_square_y = 0;
    }

    global_velocity_x += acceleration_x * dt_s;
    global_velocity_y += acceleration_y * dt_s;

    // Kinematic Formula
    global_square_x += global_velocity_x * dt_s + (0.5f * acceleration_x * dt_s * dt_s);
    global_square_y += global_velocity_y * dt_s + (0.5f * acceleration_y * dt_s * dt_s);

    printf("x: %.2f, y: %.2f\n", global_square_x, global_square_y);

    real32 square_size = 10;

    // Clear the screen
    SDL_SetRenderDrawColor(global_renderer, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(global_renderer);

    // Draw canvas that perfectly reflects aspect ratio
    real32 canvas_width;
    real32 canvas_height;

    if ((real32)window_width / (real32)window_height > ABSOLUTE_ASPECT_RATIO)
    {
        // Window is wider than the desired aspect ratio
        canvas_height = (real32)window_height;
        canvas_width = canvas_height * ABSOLUTE_ASPECT_RATIO;
    }
    else
    {
        // Window is taller than the desired aspect ratio
        canvas_width = (real32)window_width;
        canvas_height = canvas_width / ABSOLUTE_ASPECT_RATIO;
    }

    real32 normalized_x = global_square_x * render_scale;
    real32 normalized_y = global_square_y * render_scale;
    /*
    // Normalize coords to screen (50% -> 0.5)
    if ((real32)window_width / (real32)window_height > ABSOLUTE_ASPECT_RATIO)
    {
        // Height is the constraining factor
        normalized_x *= canvas_height * render_scale;
        normalized_y *= canvas_height * render_scale;
        // square_size *= canvas_height * render_scale;
    }
    else
    {
        // Width is the constraining factor
        normalized_x *= canvas_width * render_scale;
        normalized_y *= canvas_width * render_scale;
    }

    // We want coords to be (0,0) but map to the canvas coords
    normalized_x += canvas_width / 2;
    normalized_y += canvas_height / 2;
    */

    SDL_Rect drawable_canvas;
    drawable_canvas.x = (window_width - (int32)canvas_width) / 2;
    drawable_canvas.y = (window_height - (int32)canvas_height) / 2;
    drawable_canvas.w = (int32)canvas_width;
    drawable_canvas.h = (int32)canvas_height;

    // Set the clip rectangle to restrict rendering
    SDL_RenderSetClipRect(global_renderer, &drawable_canvas);

    // Set the drawing color to blue (RGBA)
    SDL_SetRenderDrawColor(global_renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(global_renderer, &drawable_canvas);

    real32 actual_x = drawable_canvas.x + (canvas_width / 2) + (normalized_x * canvas_width / 2);
    real32 actual_y =
        drawable_canvas.y + (canvas_height / 2) + (normalized_y * canvas_height / 2 * ABSOLUTE_ASPECT_RATIO);
    real32 actual_square_size = square_size * canvas_width * render_scale;

    // printf("%.2f\n", actual_x);

    SDL_Rect red_square = {};
    red_square.x = (int32)(actual_x - (actual_square_size / 2));
    red_square.y = (int32)(actual_y - (actual_square_size / 2));
    red_square.w = (int32)actual_square_size;
    red_square.h = (int32)actual_square_size;

    // Set the drawing color to red (RGBA)
    SDL_SetRenderDrawColor(global_renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(global_renderer, &red_square);

    global_angle += 90.0f * dt_s;  // Rotate 90 degrees per second

    if (global_angle >= 360.0f)
    {
        // Keep angle constrained
        global_angle -= 360.0f;
    }

    // Render the rotating square with the current angle
    {
        SDL_Rect dst_rect = {};
        dst_rect.x = (int32)(actual_x - (actual_square_size / 2));
        dst_rect.y = (int32)(actual_y - (actual_square_size / 2));
        dst_rect.w = (int32)actual_square_size;
        dst_rect.h = (int32)actual_square_size;

        // Center of the square for rotation
        SDL_Point center = {(int32)(actual_square_size / 2), (int32)(actual_square_size / 2)};

        // Render the rotating square using SDL_RenderCopyEx
        SDL_RenderCopyEx(
            global_renderer, global_square_texture, nullptr, &dst_rect, global_angle, &center, SDL_FLIP_NONE
        );
    }

    // Present the rendered content
    SDL_RenderPresent(global_renderer);
}