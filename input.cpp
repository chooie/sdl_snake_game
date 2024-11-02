#define is_down(b) input->buttons[b].is_down
#define pressed(b) input->buttons[b].is_down && input->buttons[b].changed
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
                    process_input(BUTTON_SPACE, SDLK_SPACE);
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
                    case SDLK_ESCAPE:
                    {
                        global_running = 0;
                    }
                    break;
                    // case SDLK_s:
                    // {
                    //     SDL_WarpMouseInWindow(global_window, window_width / 2, window_height / 2);
                    // } break;
                    case SDLK_f:
                    {
                        int isFullScreen = SDL_GetWindowFlags(global_window) & SDL_WINDOW_FULLSCREEN;
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
                            SDL_SetWindowFullscreen(global_window, SDL_WINDOW_FULLSCREEN);
                            SDL_ShowCursor(SDL_DISABLE);
                            SDL_SetRelativeMouseMode(SDL_TRUE);
                        }

                        // TODO: this is probably a memory leak. Need to destroy the texture every time we recreate it
                        global_square_texture = createSquareTexture(global_renderer, window_width / 4);
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
                global_running = false;
            }
            break;
        }
    }

    if (pressed(BUTTON_SPACE))
    {
        global_paused = !global_paused;
    }
}