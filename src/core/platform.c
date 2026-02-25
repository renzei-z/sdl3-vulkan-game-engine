#include <SDL3/SDL.h>
#include <core/platform.h>

bool platform_poll_events(engine_event *out) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT:
                out->type = ENGINE_EVENT_QUIT;
                return true;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                out->type = ENGINE_EVENT_RESIZE;
                return true;
            default:
                out->type = ENGINE_EVENT_NONE;
                break;
        }
    }
    return false;
}
