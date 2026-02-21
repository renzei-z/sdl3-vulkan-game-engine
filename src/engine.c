#include <engine.h>

#include <stdlib.h>

#ifdef __VK_BACKEND

void engine_init(engine_state *e, const char *title, int width, int height) {
    vk_context_init(
        &e->vk,
        title,
        width,
        height);
    e->running = true;

    vk_pipeline_config cfg = vk_default_pipeline_config();

    cfg.layout = e->vk.pipeline_layout;
    cfg.render_pass = e->vk.render_pass;
    
    vk_pipeline_build(&e->vk, "shaders/tri-vert.spv", "shaders/tri-frag.spv", &cfg);
}

[[noreturn]] void engine_quit(engine_state *e) {
    vk_context_shutdown(&e->vk);
    exit(0);
}

#endif // __VK_BACKEND
