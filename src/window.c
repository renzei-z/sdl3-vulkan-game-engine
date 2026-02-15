#include <SDL3/SDL_video.h>
#include <window.h>

#include <SDL3/SDL.h>

#include <util/logger.h>

#ifdef __VK_BACKEND
    #define FLAGS SDL_WINDOW_VULKAN
#else
    #define FLAGS 0
#endif // __VK_BACKEND

void window_init(window *w, const char *title, int width, int height) {
    check_sdl_result(
        SDL_Init(SDL_INIT_VIDEO),
        "Could not initialize SDL"
    );

    w->window = SDL_CreateWindow(title, width, height, SDL_WINDOW_VULKAN);
    check_sdl_result(
        w->window, 
        "Could not create SDL window"
    );
}

void window_shutdown(window *w) {
    SDL_DestroyWindow(w->window);
    SDL_Quit();
}