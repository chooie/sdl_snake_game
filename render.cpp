#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

struct Color_RGBA
{
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 alpha;
};

void draw_rect(SDL_Rect rect, SDL_Color color) {
    SDL_SetRenderDrawColor(global_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(global_renderer, &rect);
}

void render_text_with_scaling(const char* text, int32 x, int32 y, real32 font_size, SDL_Color text_color)
{
    int32 pt_size = (int32)(0.5f + font_size * global_text_dpi_scale_factor);
    // printf("%d\n", pt_size);
    TTF_Font* font = TTF_OpenFont("fonts/Roboto/Roboto-Medium.ttf", pt_size);
    if (font == nullptr)
    {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, text_color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(global_renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect rect = {};

    TTF_SizeText(font, text, &rect.w, &rect.h);

    real32 text_aspect_ratio = (real32)rect.w / (real32)rect.h;

    rect.w = (int32)rect.w;
    rect.h = (int32)(rect.w / text_aspect_ratio);

    rect.w /= 2;
    rect.h /= 2;

    rect.x = x;
    rect.y = y;

    SDL_RenderCopy(global_renderer, texture, NULL, &rect);

    // Cleanup
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
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
    rect.h = (int32)(desired_width / text_aspect_ratio);

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

Screen_Space_Position
map_world_space_position_to_screen_space_position(real32 world_x, real32 world_y)
{
    Screen_Space_Position result = {};
    real32 actual_x = world_x * GRID_BLOCK_SIZE;
    // Flip Y so positive y is up and negative y is down
    real32 actual_y = LOGICAL_HEIGHT - (world_y * GRID_BLOCK_SIZE);

    // printf("x: %.2f, y: %.2f\n", actual_x, actual_y);

    result.x = actual_x;
    result.y = actual_y;
    return result;
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
        uint32 border_thickness = 1;                // Thickness of the white border
        SDL_Color grey_color = {40, 40, 40, 255};   // Dark grey color
        SDL_Color white_color = {60, 60, 60, 255};  // Lighter color for borders

        for (uint32 row_index = 0; row_index < Y_GRIDS; row_index++)
        {
            for (uint32 column_index = 0; column_index < X_GRIDS; column_index++)
            {
                SDL_Rect grid_block = {};
                grid_block.x = (int32)(column_index * GRID_BLOCK_SIZE);
                grid_block.y = (int32)(row_index * GRID_BLOCK_SIZE);
                grid_block.w = (int32)(GRID_BLOCK_SIZE);
                grid_block.h = (int32)(GRID_BLOCK_SIZE);

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

    { // Draw Blip
        Screen_Space_Position square_screen_pos =
            map_world_space_position_to_screen_space_position(state->blip_pos_x, state->blip_pos_y);

        real32 size = GRID_BLOCK_SIZE * 0.5f;

        SDL_Rect square = {};
        square.x = (int32)(square_screen_pos.x + ((real32)GRID_BLOCK_SIZE / 2) - (size / 2));
        square.y = (int32)(square_screen_pos.y + ((real32)GRID_BLOCK_SIZE / 2) - (size / 2));
        square.w = (int32)size;
        square.h = (int32)size;

        SDL_Color color = {52, 152, 219, 255};
        draw_rect(square, color);
    }

    { // Draw Player
        Screen_Space_Position square_screen_pos =
            map_world_space_position_to_screen_space_position(state->pos_x, state->pos_y);

        SDL_Rect square = {};
        square.x = (int32)(square_screen_pos.x);
        square.y = (int32)(square_screen_pos.y);
        square.w = (int32)GRID_BLOCK_SIZE;
        square.h = (int32)GRID_BLOCK_SIZE;

        SDL_Color red = {171, 70, 66, 255};
        draw_rect(square, red);

        for (uint32 i = 0; i < MAX_TAIL_LENGTH; i++)
        {
            Snake_Part* snake_part = &state->snake_parts[i];
            Screen_Space_Position screen_pos =
                map_world_space_position_to_screen_space_position(snake_part->pos_x, snake_part->pos_y);

            SDL_Rect square = {};
            square.x = (int32)(screen_pos.x);
            square.y = (int32)(screen_pos.y);
            square.w = (int32)GRID_BLOCK_SIZE;
            square.h = (int32)GRID_BLOCK_SIZE;

            SDL_Color darkened_red = {154, 63, 59, 255};
            draw_rect(square, darkened_red);
        }
    }
}