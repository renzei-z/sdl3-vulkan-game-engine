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

  vertex *vertex_buffer_cpu;
  uint32_t vertex_count;
  
  bool running;
} engine_state;

void engine_init(engine_state *e, const char *title, int width, int height);

void engine_draw_triangle(engine_state *e, vertex v1, vertex v2, vertex v3);

[[noreturn]] void engine_quit(engine_state *e);

#endif // ENGINE_H_
