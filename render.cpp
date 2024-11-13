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

#define MAX_FONTS 100  // Define a maximum number of cached fonts

// Define a struct to represent a key-value pair
typedef struct {
    int pt_size;
    TTF_Font* font;
} FontEntry;

FontEntry global_font_cache[MAX_FONTS];
int global_font_cache_size = 0;  // Number of entries in the cache

TTF_Font* get_font(int32 pt_size)
{
    // Search for the font in the cache
    for (uint32 i = 0; i < global_font_cache_size; ++i)
    {
        if (global_font_cache[i].pt_size == pt_size)
        {
            return global_font_cache[i].font;  // Found in cache
        }
    }

    // Font not found in cache, so load it
    TTF_Font* font = TTF_OpenFont("fonts/Share_Tech_Mono/ShareTechMono-Regular.ttf", pt_size);
    if (!font)
    {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        return NULL;
    }

    // Add the new font to the cache if there's space
    if (global_font_cache_size < MAX_FONTS)
    {
        global_font_cache[global_font_cache_size].pt_size = pt_size;
        global_font_cache[global_font_cache_size].font = font;
        global_font_cache_size++;
    }
    else
    {
        fprintf(stderr, "Font cache is full!\n");
    }

    return font;
}

// Cleanup function to free all fonts in the cache
void cleanup_fonts()
{
    for (int i = 0; i < global_font_cache_size; ++i)
    {
        TTF_CloseFont(global_font_cache[i].font);
    }
    global_font_cache_size = 0;
}

void render_text_with_scaling(const char* text,
                              int32 x, int32 y,
                              real32 font_size,
                              SDL_Color text_color,
                              SDL_Rect* return_rect)
{
    int32 pt_size = (int32)(0.5f + font_size * global_text_dpi_scale_factor);
    // printf("%d\n", pt_size);
    TTF_Font* font = get_font(pt_size);

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, text_color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(global_renderer, surface);
    // Pass-through the color's alpha channel to control opacity
    SDL_SetTextureAlphaMod(texture, text_color.a);
    SDL_FreeSurface(surface);

    return_rect->x = x;
    return_rect->y = y;
    SDL_QueryTexture(texture, NULL, NULL, &return_rect->w, &return_rect->h);

    return_rect->w /= global_text_dpi_scale_factor;
    return_rect->h /= global_text_dpi_scale_factor;

    SDL_RenderCopy(global_renderer, texture, NULL, return_rect);

    // Cleanup
    SDL_DestroyTexture(texture);
}

void render_text_with_scaling(const char* text,
                              int32 x, int32 y,
                              real32 font_size,
                              SDL_Color text_color)
{
    SDL_Rect rect = {};
    render_text_with_scaling(text, x, y, font_size, text_color, &rect);
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

SDL_Texture* grid_texture = NULL;
int grid_texture_initialized = 0;

// Function to draw the grid onto a texture for caching
void create_grid_texture(SDL_Renderer* renderer)
{
    uint32 border_thickness = 1;                // Thickness of the white border
    SDL_Color grey_color = {40, 40, 40, 255};   // Dark grey color
    SDL_Color white_color = {60, 60, 60, 255};  // Lighter color for borders

    // Calculate the grid texture size
    int grid_width = X_GRIDS * GRID_BLOCK_SIZE;
    int grid_height = Y_GRIDS * GRID_BLOCK_SIZE;

    // Create the texture
    grid_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, grid_width, grid_height);
    if (!grid_texture)
    {
        fprintf(stderr, "Failed to create grid texture: %s\n", SDL_GetError());
        return;
    }

    // Set the grid texture as the render target
    SDL_SetRenderTarget(renderer, grid_texture);

    // Draw the grid
    for (uint32 row_index = 0; row_index < Y_GRIDS; row_index++)
    {
        for (uint32 column_index = 0; column_index < X_GRIDS; column_index++)
        {
            SDL_Rect grid_block;
            grid_block.x = (int32)(column_index * GRID_BLOCK_SIZE);
            grid_block.y = (int32)(row_index * GRID_BLOCK_SIZE);
            grid_block.w = (int32)(GRID_BLOCK_SIZE);
            grid_block.h = (int32)(GRID_BLOCK_SIZE);

            // Draw the white border rectangle
            SDL_SetRenderDrawColor(renderer, white_color.r, white_color.g, white_color.b, white_color.a);
            SDL_RenderFillRect(renderer, &grid_block);

            // Shrink the grey rectangle by the border thickness to draw it inside the border
            SDL_Rect inner_block;
            inner_block.x = (int32)(grid_block.x + border_thickness);
            inner_block.y = (int32)(grid_block.y + border_thickness);
            inner_block.w = (int32)(grid_block.w - (2 * border_thickness));
            inner_block.h = (int32)(grid_block.h - (2 * border_thickness));

            // Draw the dark grey fill within the border
            SDL_SetRenderDrawColor(renderer, grey_color.r, grey_color.g, grey_color.b, grey_color.a);
            SDL_RenderFillRect(renderer, &inner_block);
        }
    }

    // Reset the rendering target to the default (screen)
    SDL_SetRenderTarget(renderer, NULL);

    grid_texture_initialized = 1;
}

// Function to render the grid by reusing the cached texture
void render_grid(SDL_Renderer* renderer)
{
    // Create the grid texture if it hasn't been created yet
    if (!grid_texture_initialized)
    {
        create_grid_texture(renderer);
    }

    // Render the cached grid texture to the screen
    SDL_RenderCopy(renderer, grid_texture, NULL, NULL);
}

void render(State* state)
{
    // NOTE: We need this to distinguish the 'usable canvas' from the black dead-space (due to differing aspect ratios)
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

    render_grid(global_renderer);

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

        for (uint32 i = 0; i < state->next_snake_part_index; i++)
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