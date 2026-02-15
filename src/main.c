#ifndef __VK_BACKEND
    #define __VK_BACKEND
#endif // __VK_BACKEND

#include <engine.h>
#include <platform.h>

#include <SDL3/SDL_log.h>

int main(void) {
    engine_state engine = {0};
    engine_init(&engine, "[GAME] Game Engine", 800, 400);

    while (engine.running) {
        engine_event e;
        while (engine.running && platform_poll_events(&e)) {
            switch (e.type) {
                case ENGINE_EVENT_QUIT:
                    SDL_Log("[EVENT] Quit.\n");
                    engine.running = false;
                    break;
                case ENGINE_EVENT_RESIZE:
                    engine.vk.framebuffer_resized = true;
                    break;
                default:
                    break;
            }
        }

        vk_draw_frame(&engine.vk);
    }

    engine_quit(&engine);
}