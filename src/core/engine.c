#include <core/engine.h>

#include <stdlib.h>

#include <SDL3/SDL_log.h>

#ifdef __VK_BACKEND

#include <vk_mem_alloc.h>

void engine_init(engine_state *e, const char *title, int width, int height) {
    vk_context_init(
        &e->vk,
        title,
        width,
        height);
    e->running = true;

    // TODO: Technically, in __vk_vma_create_allocation, we set MAPPED,
    // so this is redundant. We should just get the returned pointer that is
    // already mapped through a VmaAllocationInfo struct.
    vmaMapMemory(e->vk.allocator, e->vk.allocation, (void**)&e->vertex_map);
    e->vertex_count = 0;
    
    vk_pipeline_config cfg = vk_default_pipeline_config();

    cfg.layout = e->vk.pipeline_layout;
    cfg.render_pass = e->vk.render_pass;

    // TODO: This should be called by the program itself to load a shader.
    // Hardcoding a shader here is not good practice.
    e->vk.tri_pipeline = vk_pipeline_build(&e->vk, "shaders/tri-vert.spv", "shaders/tri-frag.spv", &cfg);
}

void engine_begin_frame(engine_state *e) {
  e->vertex_count = 0;
}

void engine_draw_triangle_basic(engine_state *e, float x1, float y1, float x2, float y2, float x3, float y3) {
  vertex vertices[3] = {
    {{x1, y1, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{x2, y2, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{x3, y3, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
  };

  engine_draw_triangle(e, vertices[0], vertices[1], vertices[2]);
}

void engine_draw_triangle(engine_state *e, vertex v1, vertex v2, vertex v3) {
  if (e->vertex_count + 3 > MAX_VERTICES) {
    SDL_Log("[WARNING] Attempted to draw a triangle, but exceeded MAX_VERTICES.\n");
    return;
  }

  if (e->vertex_map == NULL) {
    SDL_Log("[WARNING] Attempted to draw a triangle, but GPU vertex mapping doesn't exist.\n");
    return;
  }

  e->vertex_map[e->vertex_count + 0] = v1;
  e->vertex_map[e->vertex_count + 1] = v2;
  e->vertex_map[e->vertex_count + 2] = v3;

  e->vertex_count += 3;
}

void engine_do_render(engine_state *e) {
  vk_draw_frame(&e->vk, e->vertex_count);
}

[[noreturn]] void engine_quit(engine_state *e) {
    vk_context_shutdown(&e->vk);
    exit(0);
}

#endif // __VK_BACKEND
