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
    // Flip Y so positive y is up and negative y is down
    real32 actual_y = LOGICAL_HEIGHT - (percent_y * (LOGICAL_WIDTH / 2) + (LOGICAL_HEIGHT / 2));

    result.x = actual_x;
    result.y = actual_y;
    return result;
}

real32
map_world_space_size_to_screen_space_size(real32 size)
{
    return size * RENDER_SCALE * LOGICAL_WIDTH;
}

void render(State* state)
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

#if 1
    // TODO: This is really slow
    {  // Draw Grid

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

        uint32 grid_block_size = 20;
        uint32 border_thickness = 1;                       // Thickness of the white border
        uint32 x_grids = LOGICAL_WIDTH / grid_block_size;  // Should exactly divide into logical width
        uint32 y_grids = (int32)(x_grids / ABSOLUTE_ASPECT_RATIO);

        SDL_Color grey_color = {40, 40, 40, 255};   // Dark grey color
        SDL_Color white_color = {60, 60, 60, 255};  // Lighter color for borders

        for (uint32 row_index = 0; row_index < y_grids; row_index++)
        {
            for (uint32 column_index = 0; column_index < x_grids; column_index++)
            {
                SDL_Rect grid_block = {};
                grid_block.x = (int32)(column_index * grid_block_size);
                grid_block.y = (int32)(row_index * grid_block_size);
                grid_block.w = (int32)(grid_block_size);
                grid_block.h = (int32)(grid_block_size);

                // Draw the white border rectangle
                draw_rect(grid_block, white_color);

                // Shrink the grey rectangle by the border thickness to draw it inside the border
                SDL_Rect inner_block;
                inner_block.x = (int32)(grid_block.x + border_thickness);
                inner_block.y = (int32)(grid_block.y + border_thickness);
                inner_block.w = (int32)(grid_block.w - (2 * border_thickness));
                inner_block.h = (int32)(grid_block.h - (2 * border_thickness));

                // Draw the dark grey fill within the border
                draw_rect(inner_block, grey_color);
            }
        }
    }
#endif

    { // Draw Player
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
    }
}