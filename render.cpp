#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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
void draw_square_old(SDL_Rect drawable_canvas,
                     real32 world_space_x, real32 world_space_y,
                     real32 world_space_size,
                     Color_RGBA color)
{
    real32 normalized_x = world_space_x * RENDER_SCALE;
    real32 normalized_y = world_space_y * RENDER_SCALE;
    real32 actual_x = normalized_x * LOGICAL_WIDTH + (LOGICAL_WIDTH / 2);
    real32 actual_y = normalized_y * LOGICAL_WIDTH + (LOGICAL_HEIGHT / 2);
    real32 actual_square_size = world_space_size * RENDER_SCALE * LOGICAL_WIDTH;

    SDL_Rect square = {};
    square.x = (int32)(actual_x - (actual_square_size / 2));
    square.y = (int32)(actual_y - (actual_square_size / 2));
    square.w = (int32)actual_square_size;
    square.h = (int32)actual_square_size;

    SDL_SetRenderDrawColor(global_renderer, color.red, color.green, color.blue, color.alpha);
    SDL_RenderFillRect(global_renderer, &square);
}

void draw_rect(SDL_Rect rect, SDL_Color color) {
    SDL_SetRenderDrawColor(global_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(global_renderer, &rect);
}

// Function to create a text texture
SDL_Texture* createTextTexture(SDL_Renderer* renderer,
                               TTF_Font* font,
                               const char* text,
                               SDL_Color color,
                               SDL_Rect& text_rect,
                               real32 world_space_size)
{
    // Create a surface for the text
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    TTF_SizeText(global_font, text, &text_rect.w, &text_rect.h);
    SDL_FreeSurface(textSurface);

    return textTexture;
}

// Usage in render function:
void render_text(State* state,
                 SDL_Rect drawable_canvas,
                 const char* message,
                 TTF_Font* font,
                 real32 world_space_x,
                 real32 world_space_y,
                 real32 world_space_size,
                 SDL_Color text_color)
{
    SDL_Rect text_rect = {};
     // Calculate the actual position based on world space
    real32 normalized_x = world_space_x * RENDER_SCALE;
    real32 normalized_y = world_space_y * RENDER_SCALE;

    // TTF_SetFontOutline(font, 4);

    SDL_Texture* textTexture =
        createTextTexture(global_renderer, font, message, text_color, text_rect, world_space_size);

    // Scale text to match world space size
    real32 scale_factor = world_space_size * RENDER_SCALE * ((real32)drawable_canvas.w / text_rect.w);
    text_rect.w = (int32)(text_rect.w * scale_factor);
    text_rect.h = (int32)(text_rect.h * scale_factor);

    text_rect.x =
        drawable_canvas.x + (drawable_canvas.w / 2) + (int32)(normalized_x * drawable_canvas.w / 2) - (text_rect.w / 2);
    text_rect.y = drawable_canvas.y + (drawable_canvas.h / 2) +
                  (int32)(normalized_y * drawable_canvas.h / 2 * ABSOLUTE_ASPECT_RATIO) - (text_rect.h / 2);

    // Render the text texture onto the canvas
    SDL_RenderCopy(global_renderer, textTexture, nullptr, &text_rect);

    // Cleanup
    SDL_DestroyTexture(textTexture);
}

struct Screen_Space_Position {
    real32 x;
    real32 y;
};

/*
0, 0 is the origin that corresponds to 640, 360
*/
Screen_Space_Position
map_world_space_position_to_screen_space_position(real32 world_x, real32 world_y)
{
    Screen_Space_Position result = {};
    real32 percent_x = world_x * RENDER_SCALE;
    real32 percent_y = world_y * RENDER_SCALE;
    real32 actual_x = percent_x * (LOGICAL_WIDTH / 2) + (LOGICAL_WIDTH / 2);
    real32 actual_y = percent_y * (LOGICAL_WIDTH / 2) + (LOGICAL_HEIGHT / 2);

    result.x = actual_x;
    result.y = actual_y;
    return result;
}

real32
map_world_space_size_to_screen_space_size(real32 size)
{
    return size * RENDER_SCALE * LOGICAL_WIDTH;
}

void render(State* state, SDL_Texture* square_texture)
{
    // Clear the screen
    SDL_SetRenderDrawColor(global_renderer, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(global_renderer);

    SDL_Rect drawable_canvas;
    drawable_canvas.x = 0;
    drawable_canvas.y = 0;
    drawable_canvas.w = LOGICAL_WIDTH;
    drawable_canvas.h = LOGICAL_HEIGHT;

    // Set the clip rectangle to restrict rendering
    SDL_RenderSetClipRect(global_renderer, &drawable_canvas);

    // Use a neutral background color to not cause too much eye strain
    SDL_SetRenderDrawColor(global_renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(global_renderer, &drawable_canvas);

    Screen_Space_Position square_screen_pos =
        map_world_space_position_to_screen_space_position(state->pos_x, state->pos_y);
    real32 square_size = map_world_space_size_to_screen_space_size(10.0f);

    SDL_Rect square = {};
    square.x = (int32)(square_screen_pos.x - (square_size / 2));
    square.y = (int32)(square_screen_pos.y - (square_size / 2));
    square.w = (int32)square_size;
    square.h = (int32)square_size;

    SDL_Color red = {171, 70, 66, 255};
    draw_rect(square, red);

    Screen_Space_Position other_square_screen_pos =
        map_world_space_position_to_screen_space_position(50.0f, 0);
    real32 other_square_size = map_world_space_size_to_screen_space_size(10.0f);

    SDL_Rect other_square = {};
    other_square.x = (int32)(other_square_screen_pos.x - (other_square_size / 2));
    other_square.y = (int32)(other_square_screen_pos.y - (other_square_size / 2));
    other_square.w = (int32)other_square_size;
    other_square.h = (int32)other_square_size;

    SDL_Color magenta = {186, 139, 175, 255};
    draw_rect(other_square, magenta);


    Screen_Space_Position rotating_square_screen_pos =
        map_world_space_position_to_screen_space_position(state->pos_x, state->pos_y);
    real32 rotating_square_size = map_world_space_size_to_screen_space_size(10.0f);

    SDL_Rect rotating_square = {};
    rotating_square.x = (int32)(rotating_square_screen_pos.x - (rotating_square_size / 2));
    rotating_square.y = (int32)(rotating_square_screen_pos.y - (rotating_square_size / 2));
    rotating_square.w = (int32)rotating_square_size;
    rotating_square.h = (int32)rotating_square_size;

    // Center of the square for rotation
    SDL_Point center = {(int32)(rotating_square_size / 2), (int32)(rotating_square_size / 2)};

    // Render the rotating square using SDL_RenderCopyEx
    SDL_RenderCopyEx(global_renderer,
                     square_texture,
                     nullptr,
                     &rotating_square,
                     state->angle,
                     &center,
                     SDL_FLIP_NONE);
    /*

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
    
    // Example text rendering
    SDL_Color white = {255, 255, 255, 255};
    const char* message = "Hello, World!";
    real32 text_world_x = 0;   // X position in world space
    real32 text_world_y = 0;   // Y position in world space
    real32 text_world_size = 50;  // World space size of text

    render_text(state, drawable_canvas, message, global_font, text_world_x, text_world_y, text_world_size, white);
    */

    SDL_RenderCopy(global_renderer, global_text_texture, nullptr, &global_text_rect);

    // Present the rendered content
    SDL_RenderPresent(global_renderer);
}