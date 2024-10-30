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

struct Color_RGBA
{
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 alpha;
};

real32 RENDER_SCALE = 0.01f;

/*
Draw from world space to canvas in window
Coords are in -100 to 100 space in x direction
Because the aspect ratio is 16 / 9, coords are -56.25 to 56.25 in y direction
*/
void draw_square(SDL_Rect drawable_canvas,
                 real32 world_space_x, real32 world_space_y,
                 real32 world_space_size,
                 Color_RGBA color)
{
    real32 normalized_x = world_space_x * RENDER_SCALE;
    real32 normalized_y = world_space_y * RENDER_SCALE;

    real32 actual_x = drawable_canvas.x +
                      (drawable_canvas.w / 2) +
                      (normalized_x * drawable_canvas.w / 2);
    real32 actual_y = drawable_canvas.y +
                      (drawable_canvas.h / 2) +
                      // Account for width being wider than height
                      (normalized_y * drawable_canvas.h / 2 * ABSOLUTE_ASPECT_RATIO);
    real32 actual_square_size = world_space_size * RENDER_SCALE * drawable_canvas.w;

    SDL_Rect red_square = {};
    red_square.x = (int32)(actual_x - (actual_square_size / 2));
    red_square.y = (int32)(actual_y - (actual_square_size / 2));
    red_square.w = (int32)actual_square_size;
    red_square.h = (int32)actual_square_size;

    // Set the drawing color to red (RGBA)
    SDL_SetRenderDrawColor(global_renderer, color.red, color.green, color.blue, color.alpha);
    SDL_RenderFillRect(global_renderer, &red_square);
}

void render(State* state, SDL_Texture* square_texture)
{
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

    SDL_Rect drawable_canvas;
    drawable_canvas.x = (window_width - (int32)canvas_width) / 2;
    drawable_canvas.y = (window_height - (int32)canvas_height) / 2;
    drawable_canvas.w = (int32)canvas_width;
    drawable_canvas.h = (int32)canvas_height;

    // Set the clip rectangle to restrict rendering
    SDL_RenderSetClipRect(global_renderer, &drawable_canvas);

    // Use a neutral background color to not cause too much eye strain
    SDL_SetRenderDrawColor(global_renderer, 56, 56, 56, 255);
    SDL_RenderFillRect(global_renderer, &drawable_canvas);

    Color_RGBA red = { 171, 70, 66, 255 };
    draw_square(drawable_canvas, state->pos_x, state->pos_y, 10, red);

    // Render the rotating square with the current angle
    {
        real32 normalized_x = state->pos_x * RENDER_SCALE;
        real32 normalized_y = state->pos_y * RENDER_SCALE;

        real32 actual_x = drawable_canvas.x +
                        (drawable_canvas.w / 2) +
                        (normalized_x * drawable_canvas.w / 2);
        real32 actual_y = drawable_canvas.y +
                        (drawable_canvas.h / 2) +
                        // Account for width being wider than height
                        (normalized_y * drawable_canvas.h / 2 * ABSOLUTE_ASPECT_RATIO);
        real32 actual_square_size = 10 * RENDER_SCALE * drawable_canvas.w;

        SDL_Rect dst_rect = {};
        dst_rect.x = (int32)(actual_x - (actual_square_size / 2));
        dst_rect.y = (int32)(actual_y - (actual_square_size / 2));
        dst_rect.w = (int32)actual_square_size;
        dst_rect.h = (int32)actual_square_size;

        // Center of the square for rotation
        SDL_Point center = {(int32)(actual_square_size / 2), (int32)(actual_square_size / 2)};

        // Render the rotating square using SDL_RenderCopyEx
        SDL_RenderCopyEx(global_renderer,
                         square_texture,
                         nullptr,
                         &dst_rect,
                         state->angle,
                         &center,
                         SDL_FLIP_NONE);
    }

    Color_RGBA magenta = { 186, 139, 175, 255 };
    draw_square(drawable_canvas, 75, 0, 10, magenta);

    // Present the rendered content
    SDL_RenderPresent(global_renderer);
}