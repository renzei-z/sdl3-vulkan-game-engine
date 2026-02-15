#ifndef ENGINE_H_
#define ENGINE_H_

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
    bool running;
} engine_state;

void engine_init(engine_state *e, const char *title, int width, int height);

[[noreturn]] void engine_quit(engine_state *e);

#endif // ENGINE_H_