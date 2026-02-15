#ifndef LOGGER_H_
#define LOGGER_H_

#include <SDL3/SDL_log.h>

void check_sdl_result(bool res, const char *msg);
bool check_mem_alloc(void *ptr);

#ifdef __VK_BACKEND
#include <vulkan/vulkan_core.h>

void check_vk_result(VkResult res, const char *msg);

#endif // __VK_BACKEND

#endif // LOGGER_H_