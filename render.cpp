#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

struct Color_RGBA
{
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 alpha;
};

real32 RENDER_SCALE = 0.01f;

void draw_rect(SDL_Rect rect, SDL_Color color) {
    SDL_SetRenderDrawColor(global_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(global_renderer, &rect);
}

void render_centered_text_with_scaling(const char* text, int32 x, int32 y, real32 desired_width, SDL_Color text_color)
{   
    SDL_Surface* surface = TTF_RenderText_Blended(global_font, text, text_color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(global_renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect rect = {};

    TTF_SizeText(global_font, text, &rect.w, &rect.h);

    real32 text_aspect_ratio = (real32)rect.w / (real32)rect.h;
    rect.w = (int32)desired_width;
    rect.h = int32(desired_width / text_aspect_ratio);

    rect.x = x;
    rect.y = y;

    // Center
    rect.x -= rect.w / 2;
    rect.y -= rect.h / 2;

    SDL_RenderCopy(global_renderer, texture, NULL, &rect);

    // Cleanup
    SDL_DestroyTexture(texture);
}

void render_text_no_scaling(const char* text, int32 x, int32 y, SDL_Color text_color)
{   // NOTE: text is left-aligned
    
    SDL_Surface* surface = TTF_RenderText_Blended(global_debug_font, text, text_color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(global_renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect rect = {};

    // TTF_SetFontSize(global_font, 512);
    TTF_SizeText(global_debug_font, text, &rect.w, &rect.h);

    rect.x = x;
    rect.y = y;

    SDL_RenderCopy(global_renderer, texture, NULL, &rect);

    // Cleanup
    SDL_DestroyTexture(texture);
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
}