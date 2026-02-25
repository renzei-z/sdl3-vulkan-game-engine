#ifndef ENGINE_H_
#define ENGINE_H_

#include <vertex.h>

// #ifndef __VK_BACKEND
// #error No appropriate backend defined. Make sure to define __VK_BACKEND before including engine.h
// #endif // __VK_BACKEND

#ifdef __VK_BACKEND
#include <vk/context.h>
#endif // __VK_BACKEND

typedef struct engine_state_t {
#ifdef __VK_BACKEND
  vk_context vk;
#endif // __VK_BACKEND

  vertex *vertex_map;
  uint32_t vertex_count;
  
  bool running;
} engine_state;

void engine_init(engine_state *e, const char *title, int width, int height);

void engine_begin_frame(engine_state *e);

void engine_draw_triangle_basic(engine_state *e, float x1, float y1, float x2, float y2, float x3, float y3);
void engine_draw_triangle(engine_state *e, vertex v1, vertex v2, vertex v3);

void engine_do_render(engine_state *e);

[[noreturn]] void engine_quit(engine_state *e);

#endif // ENGINE_H_
