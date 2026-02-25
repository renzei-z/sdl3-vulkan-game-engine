#ifndef __VK_BACKEND
    #define __VK_BACKEND
#endif // __VK_BACKEND

#include <core/engine.h>
#include <core/platform.h>

#include <math.h>
#include <SDL3/SDL.h>

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

	vertex v1 = {{ -0.25f,  0.25f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }};
	vertex v2 = {{   0.0f, -0.25f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }}; 
	vertex v3 = {{  0.25f,  0.25f }, { 0.0f, 0.5f, 0.5f }, { 0.0f, 0.0f }};

	engine_draw_triangle(&engine, v1, v2, v3);	
		
	engine_do_render(&engine);
    }

    engine_quit(&engine);
}
