#include <engine.h>
#include <window.h>

void window_init(window_context *context) {
    fail_check(
        SDL_Init(SDL_INIT_VIDEO), 
        "[ERROR] Failed to initialise SDL: %s\n");
}

void window_create(window_context *context, int width, int height) {
  context->window = SDL_CreateWindow(
    WINDOW_TITLE,
    width,
    height,
    SDL_WINDOW_VULKAN
  );
  // TODO: This method of passing window to fail_check seems inelegant.
  fail_check(context->window != NULL, "[ERROR] Failed to open a window: %s\n");
}