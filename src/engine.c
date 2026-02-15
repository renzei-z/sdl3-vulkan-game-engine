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
}

[[noreturn]] void engine_quit(engine_state *e) {
    vk_context_shutdown(&e->vk);
    exit(0);
}

#endif // __VK_BACKEND