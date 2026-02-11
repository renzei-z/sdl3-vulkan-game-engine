#ifndef SDL_SETUP_H_
#define SDL_SETUP_H_

#include <SDL3/SDL.h>

#define WINDOW_TITLE "[Game]"

typedef struct window_context_t {
  bool running;
  bool has_shutdown;
  SDL_Window *window;
} window_context;


void window_init(window_context *context);
void window_create(window_context *context, int width, int height);



#endif // SDL_SETUP_H_
