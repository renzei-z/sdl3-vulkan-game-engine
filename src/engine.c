#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <engine.h>
#include <render.h>
#include <vulkan/vulkan_core.h>
#include <window.h>
#include <util.h>

#include <stdbool.h> 
#include <stdlib.h>

static app_state state = {0};

int main(void) {
  window_init(&state.win);

  int width = 800, height = 600;
  window_create(
    &state.win,
    width,
    height
  );

  vk_create_instance(&state.vk);
  vk_create_surface(&state.win, &state.vk);

  vk_choose_physical_device(&state.vk);
  vk_query_queue_families(&state.vk);
  
  int graphics_family_idx = vk_find_graphics_queue_family_idx(&state.vk);
  if (graphics_family_idx == -1) {
    SDL_Log("[ERROR] Failed to find a queue family that supports graphics.\n");
    return 1;
  }

  VkDeviceQueueCreateInfo queue_create_info = vk_init_queue_create_info(graphics_family_idx);
  vk_create_device(&state.vk, queue_create_info);

  // VkQueue graphics_queue;
  // vkGetDeviceQueue(state.vk.device, graphics_family_idx, 0, &graphics_queue);

  // TODO: This should probably be checked when we *create* the device, not after.
  if (!vk_check_queue_family_supports_surface(&state.vk, graphics_family_idx)) {
    SDL_Log("[WARNING] Physical device does not support surface. This may cause issues in rendering correctly.\n");
  }

  vk_create_swapchain(&state.win, &state.vk);
  vk_create_render_pass(&state.vk);
  vk_create_frame_buffers(&state.vk);
  vk_create_pipeline_layout(&state.vk);

  size_t vert_size, frag_size;
  uint32_t *vert = load_shader_file("shaders/tri-vert.spv", &vert_size);
  uint32_t *frag = load_shader_file("shaders/tri-frag.spv", &frag_size);

  // TODO: We need to somehow get these to the shutdown code too.
  // Maybe storing the VkShaderModules in the vulkan_context is the
  // proper way to do this, but I'm not sue what we do when we have more
  // than just one hlsl file.
  vk_create_shader_module(&state.vk, vert, vert_size, &state.vk.vert);
  vk_create_shader_module(&state.vk, frag, frag_size, &state.vk.frag);

  free(vert);
  free(frag);

  state.running = true;
  while (state.running) {
    SDL_Event e = {0};

    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_EVENT_QUIT:
          state.running = false;
          break;
      }
    }

  }

  engine_shutdown(0);
}

void fail_check(bool predicate, const char *msg) {
  if (!predicate) {
    SDL_Log(msg, SDL_GetError());
    engine_shutdown(1);
  }
}

void engine_shutdown(int exit_code) {
  if (state.vk.device != VK_NULL_HANDLE){
    vkDeviceWaitIdle(state.vk.device);

    if (state.vk.frag != VK_NULL_HANDLE) 
      vkDestroyShaderModule(state.vk.device, state.vk.frag, NULL);
    
    if (state.vk.vert != VK_NULL_HANDLE) 
      vkDestroyShaderModule(state.vk.device, state.vk.vert, NULL);

    if (state.vk.pipeline_layout != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(state.vk.device, state.vk.pipeline_layout, NULL);
    }

    for (uint32_t i = 0; i < state.vk.swapchain_image_count && state.vk.swapchain_frame_buffers != NULL; ++i) {
      vkDestroyFramebuffer(state.vk.device, state.vk.swapchain_frame_buffers[i], NULL);
    }

    if (state.vk.render_pass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(state.vk.device, state.vk.render_pass, NULL);
    }

    for (uint32_t i = 0; i < state.vk.swapchain_image_count && state.vk.swapchain_image_views != NULL; ++i)
      vkDestroyImageView(state.vk.device, state.vk.swapchain_image_views[i], NULL);

    if (state.vk.swapchain != VK_NULL_HANDLE)
      vkDestroySwapchainKHR(state.vk.device, state.vk.swapchain, NULL);

    free(state.vk.swapchain_images);
    vkDestroyDevice(state.vk.device, NULL);
    
  }

  if (state.vk.surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(state.vk.instance, state.vk.surface, NULL);

  if (state.vk.instance != VK_NULL_HANDLE) 
    vkDestroyInstance(state.vk.instance, NULL);

  if (state.win.window != NULL)
    SDL_DestroyWindow(state.win.window);

  exit(exit_code);
}


