#ifndef ENGINE_H_
#define ENGINE_H_

#include <window.h>
#include <render.h>

typedef struct app_state_t {
  bool running;
  bool has_shutdown;
  window_context win;
  vulkan_context vk;
} app_state;

void fail_check(bool predicate, const char *msg);
[[noreturn]] void engine_shutdown(int exit_code);

#endif // ENGINE_H_