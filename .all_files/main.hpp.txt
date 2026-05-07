#pragma once
#include <SDL.h>

namespace app {
typedef struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int window_height, window_width;
} AppState;
}
