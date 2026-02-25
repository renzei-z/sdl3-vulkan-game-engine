#ifndef __VK_BACKEND
    #define __VK_BACKEND
#endif // __VK_BACKEND

#include <engine.h>
#include <engine/colors.h>

#include <platform.h>

#include <math.h>
#include <SDL3/SDL.h>

void temp_rotate(vertex *v, float angle) {
  float x_old = v->pos[0];
  float y_old = v->pos[1];
  
  v->pos[0] = x_old * cosf(angle) - y_old * sinf(angle);
  v->pos[1] = x_old * sinf(angle) + y_old * cosf(angle); 
}

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

	float time = SDL_GetTicks() / 1000.0f;
	float total_angle = time * 2.0f;

	float size = 0.3f;
	
	vertex v1 = {{ 0.0f, -size, 0.0f }, {1,0,0}, {0.5,0}};
	vertex v2 = {{ size,  size, 0.0f }, {0,1,0}, {1,1}};
	vertex v3 = {{-size,  size, 0.0f }, {0,0,1}, {0,1}};

	temp_rotate(&v1, total_angle);
	temp_rotate(&v2, total_angle);
	temp_rotate(&v3, total_angle);

	engine_draw_triangle(&engine, v1, v2, v3);	
	
	engine_draw_triangle(&engine, v1, v2, v3);
	
	engine_do_render(&engine);
    }

    engine_quit(&engine);
}
