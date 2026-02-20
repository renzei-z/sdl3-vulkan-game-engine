#include <SDL3/SDL_video.h>
#include <vk/context.h>

#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <util/logger.h>
#include <vulkan/vulkan_core.h>

void __vk_create_instance(vk_context *ctx);
void __vk_create_surface(vk_context *ctx);
void __vk_pick_physical_device(vk_context *ctx);
void __vk_create_logical_device(vk_context *ctx);
void __vk_get_device_queue(vk_context *ctx);
void __vk_create_swapchain(vk_context *ctx);
void __vk_create_image_views(vk_context *ctx);
void __vk_create_render_pass(vk_context *ctx);
void __vk_create_framebuffers(vk_context *ctx);
void __vk_create_command_pool(vk_context *ctx, VkCommandPool *target, uint32_t queue_idx);
void __vk_create_graphics_command_buffers(vk_context *ctx);
void __vk_create_sync_objects(vk_context *ctx);

void vk_context_init(vk_context *ctx, const char *title, int width,
                     int height) {
  window_init(&ctx->win, title, width, height);

  __vk_create_instance(ctx);
  __vk_create_surface(ctx);

  __vk_pick_physical_device(ctx);
  __vk_create_logical_device(ctx);
  __vk_get_device_queue(ctx);

  __vk_create_swapchain(ctx);
  __vk_create_image_views(ctx);

  __vk_create_render_pass(ctx);
  __vk_create_framebuffers(ctx);

  __vk_create_command_pool(ctx, &ctx->graphics_pool, ctx->graphics_family_idx);
  __vk_create_graphics_command_buffers(ctx);

  __vk_create_sync_objects(ctx);
}

#define VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"

bool __vk_is_khronos_validation_supported() {
  uint32_t instance_layer_count = 0;
  vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
  VkLayerProperties *layer_properties = (VkLayerProperties *)malloc(
      sizeof(VkLayerProperties) * instance_layer_count);
  vkEnumerateInstanceLayerProperties(&instance_layer_count, layer_properties);

  bool is_khronos_validation_supported = false;

  for (uint32_t i = 0; i < instance_layer_count; ++i) {
    VkLayerProperties prop = layer_properties[i];
    if (strcmp(prop.layerName, VK_LAYER_KHRONOS_VALIDATION_NAME) == 0) {
      is_khronos_validation_supported = true;
    }
  }

  free(layer_properties);

  return is_khronos_validation_supported;
}

void __vk_create_instance(vk_context *ctx) {
  const char *layers[] = {VK_LAYER_KHRONOS_VALIDATION_NAME};

  uint32_t extension_count = 0;
  const char *const *extensions =
      SDL_Vulkan_GetInstanceExtensions(&extension_count);

  SDL_Log("[INFO] Enumerating instance extensions:\n");
  for (uint32_t i = 0; i < extension_count; ++i) {
    const char *const ext = extensions[i];
    SDL_Log("\t%s\n", ext);
  }

  VkInstanceCreateInfo vk_create_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = NULL,
      .enabledExtensionCount = extension_count,
      .ppEnabledExtensionNames = extensions};

  if (__vk_is_khronos_validation_supported()) {
    vk_create_info.enabledLayerCount = 1;
    vk_create_info.ppEnabledLayerNames = layers;
  } else {
    SDL_Log("[WARNING] Khronos validation is not supported by your system. "
            "Debug information may be limited or non-existant.\n");
  }

  check_vk_result(vkCreateInstance(&vk_create_info, NULL, &ctx->instance),
                  "Failed to create Vulkan instance");
  SDL_Log("[INFO] Vulkan instance created successfully.\n");
}

void __vk_create_surface(vk_context *ctx) {
  check_sdl_result(SDL_Vulkan_CreateSurface(ctx->win.window, ctx->instance,
                                            NULL, &ctx->surface),
                   "Failed to create VkSurfaceKHR");
  SDL_Log("[INFO] Window surface created successfully.\n");
}

void __vk_pick_physical_device(vk_context *ctx) {
  uint32_t physical_device_count = 0;
  vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, NULL);
  if (physical_device_count == 0) {
    // TODO: We need to shutdown the engine on failure rather than just straight `exit(1)`.
    // Just not sure how to pass the engine from.. within the engine.
    SDL_Log("[ERROR] Failed to find physical GPUs with Vulkan support.\n");
    exit(1);
  }

  VkPhysicalDevice *physical_devices =
      (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physical_device_count);
  if (!check_mem_alloc(physical_devices)) {
    exit(1);
  }
  vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, physical_devices);

  ctx->physical_device = VK_NULL_HANDLE;
  for (uint32_t i = 0; i < physical_device_count; ++i) {
    VkPhysicalDeviceProperties pd_properties = {0};
    vkGetPhysicalDeviceProperties(physical_devices[i], &pd_properties);
    SDL_Log("[INFO] Found GPU Device: %s\n", pd_properties.deviceName);

    const char *type;
    switch (pd_properties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      type = "Discrete GPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      type = "Integrated GPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      type = "Virtual GPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      type = "CPU";
      break;
    default:
      type = "Other";
      break;
    }
    SDL_Log("\tType: %s\n", type);
    SDL_Log("\tAPI Version: %u.%u.%u\n",
            VK_API_VERSION_MAJOR(pd_properties.apiVersion),
            VK_API_VERSION_MINOR(pd_properties.apiVersion),
            VK_API_VERSION_PATCH(pd_properties.apiVersion));

    if (pd_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      ctx->physical_device = physical_devices[i];

      SDL_Log("[INFO] '%s' chosen as target physical device.\n", pd_properties.deviceName);
      break;
    }
  }

  if (ctx->physical_device == VK_NULL_HANDLE) {
    // TODO: Is it really a good idea to just fall back on device 0?
    ctx->physical_device = physical_devices[0];
    SDL_Log("[WARNING] Discrete GPU not found; falling back to first option.\n");
  }

  free(physical_devices);
}

void __vk_query_queue_families(vk_context *ctx) {
  vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical_device, &ctx->queue_family_count, NULL);
  ctx->queue_family_properties = 
    (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * ctx->queue_family_count);
  if (!check_mem_alloc(ctx->queue_family_properties)) {
    exit(1);
  }
  vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical_device, &ctx->queue_family_count, ctx->queue_family_properties);
}

void __vk_find_queue_families(vk_context *ctx) {
  ctx->graphics_family_idx = -1;
  ctx->compute_family_idx = -1;
  ctx->transfer_family_idx = -1;

  for (uint32_t i = 0; i < ctx->queue_family_count; ++i) {
    VkQueueFamilyProperties props = ctx->queue_family_properties[i];

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(ctx->physical_device, i, ctx->surface, &present_support);

    if ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) && present_support) {
      if (ctx->graphics_family_idx == -1)
        ctx->graphics_family_idx = i;
    }

    if (
      (props.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
      !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
    ) {
      ctx->compute_family_idx = i;
    }

    if (
      (props.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
      !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
    ) {
      ctx->transfer_family_idx = i;
    }
  }

  if (ctx->compute_family_idx == -1) ctx->compute_family_idx = ctx->graphics_family_idx;
  if (ctx->transfer_family_idx == -1) ctx->transfer_family_idx = ctx->graphics_family_idx;
  
  free(ctx->queue_family_properties);
  SDL_Log("[INFO] Chosen queue families:\n");
  SDL_Log("\tGraphics: %d\n", ctx->graphics_family_idx);
  SDL_Log("\tCompute: %d\n", ctx->compute_family_idx);
  SDL_Log("\tTransfer: %d\n", ctx->transfer_family_idx);
}

void __vk_create_logical_device(vk_context *ctx) {
  __vk_query_queue_families(ctx);
  __vk_find_queue_families(ctx);

  // TODO: Don't hardcode "3" as we may add video_decode/encode.
  // Also, we should check for stuff like VK_QUEUE_IGNORED
  uint32_t queue_indices[] = {
    ctx->graphics_family_idx,
    ctx->compute_family_idx,
    ctx->transfer_family_idx
  };

  uint32_t unique_indices[3];
  uint32_t unique_count = 0;

  for (uint32_t i = 0; i < 3; ++i) {
    bool not_unique = false;
    for (uint32_t j = 0; j < unique_count; ++j) {
      if (queue_indices[i] == unique_indices[j]) {
        not_unique = true;
        break;
      }
    }

    if (!not_unique) {
      unique_indices[unique_count++] = queue_indices[i];
    }
  }

  float queue_priority = 1.0f;
  VkDeviceQueueCreateInfo queue_create_infos[3] = {0};

  for (uint32_t i = 0; i < unique_count; ++i) {
    queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[i].queueFamilyIndex = unique_indices[i];
    queue_create_infos[i].queueCount = 1;
    queue_create_infos[i].pQueuePriorities = &queue_priority;
  }

  VkDeviceCreateInfo device_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = unique_count,
    .pQueueCreateInfos = queue_create_infos,
    .enabledExtensionCount = 1,
    .ppEnabledExtensionNames = (const char *[]) {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    }
  };

  check_vk_result(
    vkCreateDevice(ctx->physical_device, &device_create_info, NULL, &ctx->device),
    "Failed to create logical device"
  );

  SDL_Log("[INFO] Created logical device.\n");
}

void __vk_get_device_queue(vk_context *ctx) {
  vkGetDeviceQueue(
    ctx->device, 
    ctx->graphics_family_idx,
    0,
    &ctx->graphics_queue
  );
}

VkSurfaceFormatKHR __vk_choose_swap_surface_format(VkSurfaceFormatKHR *formats, uint32_t format_count) {
  for (uint32_t i = 0; i < format_count; ++i) {
    if (
      formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
      formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    ) return formats[i];
  }

  return formats[0];
}

VkPresentModeKHR __vk_choose_swap_present_mode(VkPresentModeKHR *present_modes, uint32_t present_mode_count) {
  for (uint32_t i = 0; i < present_mode_count; ++i) {
    if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) return present_modes[i];
  }

  // NOTE: VK_PRESENT_MODE_FIFO_KHR is *guaranteed* to be available, as per
  // the Vulkan specification: https://docs.vulkan.org/refpages/latest/refpages/source/VkPresentModeKHR.html
  return VK_PRESENT_MODE_FIFO_KHR;
}

swapchain_support_details __vk_query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
  swapchain_support_details support = {0};

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &support.caps);
  
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &support.format_count, NULL);
  if (support.format_count != 0) {
    support.formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * support.format_count);
    check_mem_alloc(support.formats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &support.format_count, support.formats);
  }

  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &support.present_mode_count, NULL);
  if (support.present_mode_count != 0) {
    support.present_modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * support.present_mode_count);
    check_mem_alloc(support.present_modes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &support.present_mode_count, support.present_modes);
  }

  return support;
}

void __vk_get_swapchain_extent(vk_context *ctx) {
  if (ctx->swapchain_support.caps.currentExtent.width != 0xFFFFFFFF) {
    ctx->swapchain_extent = ctx->swapchain_support.caps.currentExtent;
  } else {
    int w, h;
    SDL_GetWindowSizeInPixels(ctx->win.window, &w, &h);
    ctx->swapchain_extent.width = SDL_clamp(w, ctx->swapchain_support.caps.minImageExtent.width, ctx->swapchain_support.caps.maxImageExtent.width);
    ctx->swapchain_extent.height = SDL_clamp(h, ctx->swapchain_support.caps.minImageExtent.height, ctx->swapchain_support.caps.maxImageExtent.height);
  }
}

void __vk_create_swapchain(vk_context *ctx) {
  ctx->swapchain_support = __vk_query_swapchain_support(ctx->physical_device, ctx->surface);
  
  VkSurfaceFormatKHR format = __vk_choose_swap_surface_format(ctx->swapchain_support.formats, ctx->swapchain_support.format_count);
  VkPresentModeKHR present_mode = __vk_choose_swap_present_mode(ctx->swapchain_support.present_modes, ctx->swapchain_support.present_mode_count);

  __vk_get_swapchain_extent(ctx);

  uint32_t image_count = ctx->swapchain_support.caps.minImageCount + 1;
  if (ctx->swapchain_support.caps.maxImageCount > 0 && image_count > ctx->swapchain_support.caps.maxImageCount) {
    image_count = ctx->swapchain_support.caps.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapchain_create_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = ctx->surface,
    .minImageCount = image_count,
    .imageFormat = format.format,
    .imageColorSpace = format.colorSpace,
    .imageExtent = ctx->swapchain_extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = ctx->swapchain_support.caps.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE
  };

  check_vk_result(
    vkCreateSwapchainKHR(ctx->device, &swapchain_create_info, NULL, &ctx->swapchain),
    "Failed to create swapchain"
  );

  ctx->swapchain_format = format.format;

  vkGetSwapchainImagesKHR(ctx->device, ctx->swapchain, &ctx->image_count, NULL);
  ctx->swapchain_images = (VkImage*)malloc(sizeof(VkImage) * ctx->image_count);
  check_mem_alloc(ctx->swapchain_images);
  vkGetSwapchainImagesKHR(ctx->device, ctx->swapchain, &ctx->image_count, ctx->swapchain_images);

  free(ctx->swapchain_support.formats);
  free(ctx->swapchain_support.present_modes);

  SDL_Log("[INFO] Created swapchain.\n");
}

void __vk_create_image_views(vk_context *ctx) {
  ctx->swapchain_image_views = (VkImageView*)malloc(sizeof(VkImageView) * ctx->image_count);
  check_mem_alloc(ctx->swapchain_image_views);

  SDL_Log("[INFO] Creating image views:\n");

  for (uint32_t i = 0; i < ctx->image_count; ++i) {
    VkImageViewCreateInfo view_create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = ctx->swapchain_images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = ctx->swapchain_format,
      .components = {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .g = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .b = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .a = VK_COMPONENT_SWIZZLE_IDENTITY, 
      },
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    check_vk_result(
      vkCreateImageView(ctx->device, &view_create_info, NULL, &ctx->swapchain_image_views[i]),
      "Failed to create image view."
    );
    SDL_Log("\tImage View %u created.\n", i);
  }
}

void __vk_create_render_pass(vk_context *ctx) {
  VkAttachmentDescription color_attachment = {
    .format = ctx->swapchain_format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  };

  VkAttachmentReference color_attachment_ref = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  VkSubpassDescription subpass = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment_ref
  }; 

  VkSubpassDependency dependency = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
  };

  VkRenderPassCreateInfo render_pass_create_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &color_attachment,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 1,
    .pDependencies = &dependency
  };

  check_vk_result(
    vkCreateRenderPass(ctx->device, &render_pass_create_info, NULL, &ctx->render_pass),
    "Failed to create render pass"
  );

  SDL_Log("[INFO] Created render pass.\n");
}

void __vk_create_framebuffers(vk_context *ctx) {
  ctx->framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * ctx->image_count);
  check_mem_alloc(ctx->framebuffers);

  SDL_Log("[INFO] Creating framebuffers:\n");

  for (uint32_t i = 0; i < ctx->image_count; ++i) {
    VkImageView attachments[] = {
      ctx->swapchain_image_views[i]
    };

    VkFramebufferCreateInfo framebuffer_create_info = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = ctx->render_pass,
      .attachmentCount = 1,
      .pAttachments = attachments,
      .width = ctx->swapchain_extent.width,
      .height = ctx->swapchain_extent.height,
      .layers = 1
    };

    check_vk_result(
      vkCreateFramebuffer(ctx->device, &framebuffer_create_info, NULL, &ctx->framebuffers[i]),
      "Failed to create framebuffer."
    );

    SDL_Log("\tFramebuffer %u created.\n", i);
  }
}

void __vk_create_command_pool(vk_context *ctx, VkCommandPool *target, uint32_t queue_idx) {
  VkCommandPoolCreateInfo pool_create_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = queue_idx,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
  };

  check_vk_result(
    vkCreateCommandPool(ctx->device, &pool_create_info, NULL, target),
    "Failed to create command pool"
  );

  SDL_Log("[INFO] Created command pool for queue family %u\n", queue_idx);
}

void __vk_create_graphics_command_buffers(vk_context *ctx) {
  ctx->command_buffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);
  check_mem_alloc(ctx->command_buffers);

  VkCommandBufferAllocateInfo buffer_allocate_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = ctx->graphics_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = MAX_FRAMES_IN_FLIGHT
  }; 

  check_vk_result(
    vkAllocateCommandBuffers(ctx->device, &buffer_allocate_info, ctx->command_buffers),
    "Failed to allocate command buffers"
  );

  SDL_Log("[INFO] Allocated graphics command buffers (%u).\n", ctx->image_count);
}

void __vk_create_sync_objects(vk_context *ctx) {
  ctx->image_available_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
  check_mem_alloc(ctx->image_available_semaphores);
  
ctx->render_finished_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
  check_mem_alloc(ctx->render_finished_semaphores);

  ctx->in_flight_fences = (VkFence*)malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
  check_mem_alloc(ctx->in_flight_fences);
  
  VkSemaphoreCreateInfo semaphore_create_info = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
  };

  VkFenceCreateInfo fence_create_info = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT
  };

  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    check_vk_result(
		    vkCreateSemaphore(ctx->device, &semaphore_create_info, NULL, &ctx->image_available_semaphores[i]),
		    "Failed to create image_available semaphore"
		    );
    check_vk_result(
		    vkCreateSemaphore(ctx->device, &semaphore_create_info, NULL, &ctx->render_finished_semaphores[i]),
		    "Failed to create render_finished semaphore"
		    );
    check_vk_result(
		    vkCreateFence(ctx->device, &fence_create_info, NULL, &ctx->in_flight_fences[i]),
		    "Failed to create in_flight fence"
		    );
  }

  SDL_Log("[INFO] Created synchonization objects.\n");
}

void vk_draw_frame(vk_context *ctx) {
  vkWaitForFences(ctx->device, 1, &ctx->in_flight_fences[ctx->current_frame], VK_TRUE, UINT64_MAX);

  uint32_t img_idx;
  VkResult res = vkAcquireNextImageKHR(
    ctx->device,
    ctx->swapchain,
    UINT64_MAX,
    ctx->image_available_semaphores[ctx->current_frame],
    VK_NULL_HANDLE,
    &img_idx
  );

  if (res == VK_ERROR_OUT_OF_DATE_KHR) {
    SDL_Log("[UNHANDLED] Resize event.\n");
    abort();
  }

  vkResetFences(ctx->device, 1, &ctx->in_flight_fences[ctx->current_frame]);
  
  VkCommandBuffer cmd = ctx->command_buffers[ctx->current_frame];
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
  };
  vkBeginCommandBuffer(cmd, &begin_info);

  VkClearValue clear_color = {{{0.1f, 0.1f, 0.2f, 1.0f}}};

  VkRenderPassBeginInfo render_pass_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = ctx->render_pass,
    .framebuffer = ctx->framebuffers[img_idx],
    .renderArea = {{0, 0}, {ctx->swapchain_extent.width, ctx->swapchain_extent.height}},
    .clearValueCount = 1,
    .pClearValues = &clear_color
  };

  vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdEndRenderPass(cmd);
  vkEndCommandBuffer(cmd);

  VkPipelineStageFlags wait_stages[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };

  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &ctx->image_available_semaphores[ctx->current_frame],
    .pWaitDstStageMask = wait_stages,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &ctx->render_finished_semaphores[ctx->current_frame]
  };

  vkQueueSubmit(ctx->graphics_queue, 1, &submit_info, ctx->in_flight_fences[ctx->current_frame]);

  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &ctx->render_finished_semaphores[ctx->current_frame], // Wait for rendering to finish
    .swapchainCount = 1,
    .pSwapchains = &ctx->swapchain,
    .pImageIndices = &img_idx
  };

  vkQueuePresentKHR(ctx->graphics_queue, &present_info);

  ctx->current_frame = (ctx->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void vk_context_shutdown(vk_context *ctx) {
  if (ctx->device != VK_NULL_HANDLE)
    vkDeviceWaitIdle(ctx->device);

  if (ctx->in_flight_fences != NULL) {
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vkDestroyFence(ctx->device, ctx->in_flight_fences[i], NULL);
    }
  }
  
  if (ctx->image_available_semaphores != NULL) {
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vkDestroySemaphore(ctx->device, ctx->image_available_semaphores[i], NULL);
    }
  }
  
  if (ctx->render_finished_semaphores != NULL) {
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vkDestroySemaphore(ctx->device, ctx->render_finished_semaphores[i], NULL);
    }
  }
      
  if (ctx->graphics_pool != VK_NULL_HANDLE)
    vkDestroyCommandPool(ctx->device, ctx->graphics_pool, NULL);
  
  if (ctx->framebuffers != NULL) {
    for (uint32_t i = 0; i < ctx->image_count; ++i)
      vkDestroyFramebuffer(ctx->device, ctx->framebuffers[i], NULL);
  }
  free(ctx->framebuffers);

  if (ctx->render_pass != VK_NULL_HANDLE)
    vkDestroyRenderPass(ctx->device, ctx->render_pass, NULL);

  for (uint32_t i = 0; i < ctx->image_count; ++i) {
    // TODO: Is this safe? Image views may not have been created.
    vkDestroyImageView(ctx->device, ctx->swapchain_image_views[i], NULL); 
  }
  free(ctx->swapchain_image_views);

  if (ctx->swapchain != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(ctx->device, ctx->swapchain, NULL);

  free(ctx->swapchain_images);

  if (ctx->device != VK_NULL_HANDLE)
    vkDestroyDevice(ctx->device, NULL);

  if (ctx->surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL);

  if (ctx->instance != VK_NULL_HANDLE)
    vkDestroyInstance(ctx->instance, NULL);

  window_shutdown(&ctx->win);
}
