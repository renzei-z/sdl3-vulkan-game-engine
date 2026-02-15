#include <util/logger.h>

#include <SDL3/SDL_error.h>

#include <stdlib.h>

void check_sdl_result(bool res, const char *msg) {
    if (!res) {
        SDL_Log("[ERROR] %s: %s\n",
            msg,
            SDL_GetError());
    }
}

bool check_mem_alloc(void *ptr) {
    if (ptr == NULL) {
        SDL_Log("[ERROR] Tried to allocate, but ran out of memory.\n");
        return false;
    }
    return true;
}

#ifdef __VK_BACKEND

#include <vulkan/vk_enum_string_helper.h>

void check_vk_result(VkResult res, const char *msg) {
    if (res != VK_SUCCESS) {
        SDL_Log("[VK_ERROR] %s: %s (%d)\n",
            msg,
            string_VkResult(res),
        res);
        exit(1);
    }
}

#endif // __VK_BACKEND