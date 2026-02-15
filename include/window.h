#ifndef WINDOW_H_
#define WINDOW_H_

#include <SDL3/SDL_video.h>

typedef struct window_t {
    SDL_Window *window;
} window;

void window_init(window *w, const char *title, int width, int height);

void window_shutdown(window *w);

#endif // WINDOW_H_