#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

struct Color_RGBA
{
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 alpha;
};

void draw_rect(SDL_Rect rect, SDL_Color color)
{
    SDL_SetRenderDrawColor(global_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(global_renderer, &rect);
}

#define MAX_FONTS 100  // Define a maximum number of cached fonts

// Define a struct to represent a key-value pair
typedef struct
{
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

struct Drawn_Text
{
    char* text_string;
    real32 original_value;
    real32 font_size;
    SDL_Color color;
    SDL_Rect text_rect;
    SDL_Texture* cached_texture;
};

struct Drawn_Text_Int32
{
    char* text_string;
    int32 original_value;
    real32 font_size;
    SDL_Color color;
    SDL_Rect text_rect;
    SDL_Texture* cached_texture;
};

int32 get_font_pt_size(real32 font_size)
{
    return (int32)(0.5f + font_size * global_text_dpi_scale_factor);
}

struct Drawn_Text_Static
{
    const char* text_string;
    real32 font_size;
    SDL_Color color;
    SDL_Rect text_rect;
    SDL_Texture* cached_texture;
};

void draw_text_static(Drawn_Text_Static* drawn_text)
{
    if (!drawn_text->cached_texture)
    {
        SDL_assert(drawn_text->font_size > 0);
        int32 pt_size = get_font_pt_size(drawn_text->font_size);
        TTF_Font* font = get_font(pt_size);
        SDL_Surface* surface = TTF_RenderText_Blended(font, drawn_text->text_string, drawn_text->color);
        drawn_text->cached_texture = SDL_CreateTextureFromSurface(global_renderer, surface);
        // Pass-through the color's alpha channel to control opacity
        SDL_SetTextureAlphaMod(drawn_text->cached_texture, drawn_text->color.a);
        SDL_FreeSurface(surface);

        SDL_QueryTexture(drawn_text->cached_texture, NULL, NULL, &drawn_text->text_rect.w, &drawn_text->text_rect.h);

        drawn_text->text_rect.w /= global_text_dpi_scale_factor;
        drawn_text->text_rect.h /= global_text_dpi_scale_factor;
    }

    SDL_RenderCopy(global_renderer, drawn_text->cached_texture, NULL, &drawn_text->text_rect);
}

struct Drawn_Text_Static_2
{
    const char* text_string;
    real32 font_size;
    SDL_Color color;
    SDL_Rect text_rect;
    SDL_Texture* cached_texture;
    bool32 should_update;
};

void draw_text_static_2(Drawn_Text_Static_2* drawn_text)
{
    if (!drawn_text->cached_texture || drawn_text->should_update)
    {
        drawn_text->should_update = 0;

        if (drawn_text->cached_texture)
        {
            // Cleanup
            SDL_DestroyTexture(drawn_text->cached_texture);
            drawn_text->cached_texture = 0;
        }

        SDL_assert(drawn_text->font_size > 0);
        int32 pt_size = get_font_pt_size(drawn_text->font_size);
        TTF_Font* font = get_font(pt_size);
        SDL_Surface* surface = TTF_RenderText_Blended(font, drawn_text->text_string, drawn_text->color);
        drawn_text->cached_texture = SDL_CreateTextureFromSurface(global_renderer, surface);
        // Pass-through the color's alpha channel to control opacity
        SDL_SetTextureAlphaMod(drawn_text->cached_texture, drawn_text->color.a);
        SDL_FreeSurface(surface);

        SDL_QueryTexture(drawn_text->cached_texture, NULL, NULL, &drawn_text->text_rect.w, &drawn_text->text_rect.h);

        drawn_text->text_rect.w /= global_text_dpi_scale_factor;
        drawn_text->text_rect.h /= global_text_dpi_scale_factor;
    }

    SDL_RenderCopy(global_renderer, drawn_text->cached_texture, NULL, &drawn_text->text_rect);
}

void draw_text_real32(Drawn_Text* drawn_text, real32 current_value)
{
    if (!drawn_text->cached_texture || current_value != drawn_text->original_value)
    {
        drawn_text->original_value = current_value;

        if (drawn_text->cached_texture)
        {
            // Cleanup
            SDL_DestroyTexture(drawn_text->cached_texture);
            drawn_text->cached_texture = 0;
        }

        SDL_assert(drawn_text->font_size > 0);
        int32 pt_size = get_font_pt_size(drawn_text->font_size);
        TTF_Font* font = get_font(pt_size);
        SDL_Surface* surface = TTF_RenderText_Blended(font, drawn_text->text_string, drawn_text->color);
        drawn_text->cached_texture = SDL_CreateTextureFromSurface(global_renderer, surface);
        // Pass-through the color's alpha channel to control opacity
        SDL_SetTextureAlphaMod(drawn_text->cached_texture, drawn_text->color.a);
        SDL_FreeSurface(surface);

        SDL_QueryTexture(drawn_text->cached_texture, NULL, NULL, &drawn_text->text_rect.w, &drawn_text->text_rect.h);

        drawn_text->text_rect.w /= global_text_dpi_scale_factor;
        drawn_text->text_rect.h /= global_text_dpi_scale_factor;
    }

    SDL_RenderCopy(global_renderer, drawn_text->cached_texture, NULL, &drawn_text->text_rect);
}

void draw_text_int32(Drawn_Text_Int32* drawn_text, int32 current_value)
{
    if (!drawn_text->cached_texture || current_value != drawn_text->original_value)
    {
        drawn_text->original_value = current_value;

        if (drawn_text->cached_texture)
        {
            // Cleanup
            SDL_DestroyTexture(drawn_text->cached_texture);
            drawn_text->cached_texture = 0;
        }

        SDL_assert(drawn_text->font_size > 0);
        int32 pt_size = get_font_pt_size(drawn_text->font_size);
        TTF_Font* font = get_font(pt_size);
        SDL_Surface* surface = TTF_RenderText_Blended(font, drawn_text->text_string, drawn_text->color);
        drawn_text->cached_texture = SDL_CreateTextureFromSurface(global_renderer, surface);
        // Pass-through the color's alpha channel to control opacity
        SDL_SetTextureAlphaMod(drawn_text->cached_texture, drawn_text->color.a);
        SDL_FreeSurface(surface);

        SDL_QueryTexture(drawn_text->cached_texture, NULL, NULL, &drawn_text->text_rect.w, &drawn_text->text_rect.h);

        drawn_text->text_rect.w /= global_text_dpi_scale_factor;
        drawn_text->text_rect.h /= global_text_dpi_scale_factor;
    }

    SDL_RenderCopy(global_renderer, drawn_text->cached_texture, NULL, &drawn_text->text_rect);
}

struct Screen_Space_Position
{
    real32 x;
    real32 y;
};

Screen_Space_Position map_world_space_position_to_screen_space_position(real32 world_x, real32 world_y)
{
    Screen_Space_Position result = {};
    real32 actual_x = world_x * GRID_BLOCK_SIZE;
    // Flip Y so positive y is up and negative y is down
    real32 actual_y = LOGICAL_HEIGHT - ((world_y + 1) * GRID_BLOCK_SIZE);

    // printf("x: %.2f, y: %.2f\n", actual_x, actual_y);

    result.x = actual_x;
    result.y = actual_y;
    return result;
}

void draw_canvas()
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
}