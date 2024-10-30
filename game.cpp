#include "common.h"

real32 ACCELERATION_POWER = 1200;
real32 friction = 20.0f;

struct State {
    real32 pos_x;
    real32 pos_y;

    real32 velocity_x;
    real32 velocity_y;

    real32 angle;

    // Overload * operator for scalar multiplication
    State operator*(real32 scalar) const {
        return {pos_x * scalar, pos_y * scalar,
                velocity_x * scalar, velocity_y * scalar,
                /* TODO: how do I interpolate angles correctly? */
                angle/* * scalar */};
    }

    // Overload + operator for adding two States
    State operator+(const State& other) const {
        return {pos_x + other.pos_x, pos_y + other.pos_y,
                velocity_x + other.velocity_x, velocity_y + other.velocity_y,
                /* TODO: how do I interpolate angles correctly? */
                angle/* + other.angle*/};
    }
};

// Define your rotation speed (e.g., 90 degrees per second)
const float rotation_speed = 90.0f;

void simulate(State* state, Input* input, real64 simulation_time_elapsed, real32 dt_s)
{
    real32 acceleration_x = 0;
    real32 acceleration_y = 0;

    if (is_down(BUTTON_W))
    {
        acceleration_y += -ACCELERATION_POWER;
        // printf("Holding down W...\n");
    }

    if (is_down(BUTTON_A))
    {
        acceleration_x += -ACCELERATION_POWER;
        // printf("Holding down A...\n");
    }

    if (is_down(BUTTON_S))
    {
        acceleration_y += ACCELERATION_POWER;
        // printf("Holding down S...\n");
    }

    if (is_down(BUTTON_D))
    {
        acceleration_x += ACCELERATION_POWER;
        // printf("Holding down D...\n");
    }

    // Slow the player down through friction
    acceleration_x -= state->velocity_x * friction;
    acceleration_y -= state->velocity_y * friction;

    state->velocity_x += acceleration_x * dt_s;
    state->velocity_y += acceleration_y * dt_s;

    // Kinematic Formula
    state->pos_x += state->velocity_x * dt_s + (0.5f * acceleration_x * dt_s * dt_s);
    state->pos_y += state->velocity_y * dt_s + (0.5f * acceleration_y * dt_s * dt_s);

    state->angle += 90.0f * dt_s;  // Rotate 90 degrees per second
    if (state->angle >= 360.0f)
    {
        // Keep angle constrained
        state->angle -= 360.0f;
    }

    // printf("x: %.2f, y: %.2f\n", state->pos_x, state->pos_y);
}