#pragma once
#include <SDL.h>

namespace app {
typedef struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int window_height, window_width;

    int scroll_y = 0;
    int scroll_x = 0;
} AppState;
}
