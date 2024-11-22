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
    BUTTON_ESCAPE,

    BUTTON_COUNT,  // Should be the last item
};

struct Button_State
{
    bool32 is_down;
    bool32 changed;
};

struct Input
{
    Button_State buttons[BUTTON_COUNT];
};

#define is_down(b) input->buttons[b].is_down
#define pressed(b) input->buttons[b].is_down && input->buttons[b].changed
#define pressed_local(b) input.buttons[b].is_down && input.buttons[b].changed
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

#define process_input(button, sdl_key)                                            \
    case sdl_key:                                                                 \
    {                                                                             \
        if (event->type == SDL_KEYDOWN)                                           \
        {                                                                         \
            input->buttons[button].changed = input->buttons[button].is_down == 0; \
            input->buttons[button].is_down = 1;                                   \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            input->buttons[button].changed = input->buttons[button].is_down == 1; \
            input->buttons[button].is_down = 0;                                   \
        }                                                                         \
    }                                                                             \
    break;

void handle_input(SDL_Event* event, Input* input)
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        input->buttons[i].changed = false;
    }

    while (SDL_PollEvent(event))
    {
        switch (event->type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                switch (event->key.keysym.sym)
                {
                    process_input(BUTTON_W, SDLK_w);
                    process_input(BUTTON_A, SDLK_a);
                    process_input(BUTTON_S, SDLK_s);
                    process_input(BUTTON_D, SDLK_d);
                    process_input(BUTTON_UP, SDLK_UP);
                    process_input(BUTTON_LEFT, SDLK_LEFT);
                    process_input(BUTTON_DOWN, SDLK_DOWN);
                    process_input(BUTTON_RIGHT, SDLK_RIGHT);
                    process_input(BUTTON_SPACE, SDLK_SPACE);
                    process_input(BUTTON_ENTER, SDLK_RETURN);
                    process_input(BUTTON_ESCAPE, SDLK_ESCAPE);
                }
            }
            break;
        }

        switch (event->type)
        {
            case SDL_KEYUP:
            {
                switch (event->key.keysym.sym)
                {
                    case SDLK_BACKQUOTE:
                    {
                        global_display_debug_info = !global_display_debug_info;
                    } break;
                    case SDLK_f:
                    {
                        int isFullScreen = SDL_GetWindowFlags(global_window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
                        if (isFullScreen)
                        {
                            SDL_SetWindowFullscreen(global_window, 0);
                            SDL_ShowCursor(SDL_ENABLE);
                            SDL_SetRelativeMouseMode(SDL_FALSE);
                        }
                        else
                        {
                            // Get the desktop display mode for the main monitor (usually display index 0)
                            SDL_DisplayMode desktop_mode;
                            if (SDL_GetDesktopDisplayMode(0, &desktop_mode) != 0)
                            {
                                SDL_Log("Failed to get desktop display mode: %s", SDL_GetError());
                                return;
                            }

                            // Set the display mode to match the desktop (native) resolution and refresh rate
                            if (SDL_SetWindowDisplayMode(global_window, &desktop_mode) != 0)
                            {
                                SDL_Log("Failed to set window display mode: %s", SDL_GetError());
                            }
                            SDL_SetWindowFullscreen(global_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                            SDL_ShowCursor(SDL_DISABLE);
                            SDL_SetRelativeMouseMode(SDL_TRUE);
                        }
                    }
                    break;
                }
            }
            break;

            case SDL_WINDOWEVENT:
            {
                switch (event->window.event)
                {
                    case SDL_WINDOWEVENT_CLOSE:
                    {
                        global_running = 0;
                    }
                    break;
                }
            }
            break;

            case SDL_QUIT:
            {
                global_running = 0;
            }
            break;
        }
    }
}