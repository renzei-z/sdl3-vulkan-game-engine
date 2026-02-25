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

	engine_begin_frame(&engine);

	engine_draw_triangle_basic(&engine, -0.9f, -0.8f, -0.6f, -0.9f, -0.7f, -0.4f);
	engine_draw_triangle_basic(&engine, 0.0f, 0.0f, 0.2f, 0.3f, -0.1f, 0.4f);
	engine_draw_triangle_basic(&engine, 0.4f, 0.6f, 0.95f, -0.3f, 0.99f, 0.9f);
	
	engine_do_render(&engine);
    }

    engine_quit(&engine);
}
