real32 ACCELERATION_POWER = 10000;
real32 friction = 10.0f;

bool32 is_first_run = 1;

real32 global_velocity_x = 0;
real32 global_velocity_y = 0;

int32 global_square_x;
int32 global_square_y;

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

    // Calculate the size of the square
    int32 square_size = (window_width < window_height) ? window_width / 4 : window_height / 4;

    if (is_first_run)
    {
        is_first_run = 0;
        global_square_x = (window_width - square_size) / 2;
        global_square_y = (window_height - square_size) / 2;
    }

    global_velocity_x += acceleration_x * dt_s;
    global_velocity_y += acceleration_y * dt_s;

    // Kinematic Formula
    global_square_x += (int32)(global_velocity_x * dt_s + (0.5f * acceleration_x * dt_s * dt_s));
    global_square_y += (int32)(global_velocity_y * dt_s + (0.5f * acceleration_y * dt_s * dt_s));

    global_angle += 90.0f * dt_s;  // Rotate 90 degrees per second

    if (global_angle >= 360.0f)
    {
        // Keep angle constrained
        global_angle -= 360.0f;
    }

    // Render the rotating square with the current angle
    {
        // global_square_x += (int32)(acceleration_x * dt_s);
        // global_square_y += (int32)(acceleration_y * dt_s);

        SDL_Rect dst_rect = {global_square_x, global_square_y, square_size, square_size};

        // Clear the screen
        SDL_SetRenderDrawColor(global_renderer, 0, 0, 0, 255);  // Black background
        SDL_RenderClear(global_renderer);

        // Center of the square for rotation
        SDL_Point center = {square_size / 2, square_size / 2};

        // Render the rotating square using SDL_RenderCopyEx
        SDL_RenderCopyEx(
            global_renderer, global_square_texture, nullptr, &dst_rect, global_angle, &center, SDL_FLIP_NONE
        );

        // Present the rendered content
        SDL_RenderPresent(global_renderer);
    }
}