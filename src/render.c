#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <engine.h>
#include <render.h>
#include <vulkan/vulkan_core.h>
#include <window.h>
#include <util.h>

#include <stdlib.h>
#include <string.h>

bool is_khronos_validation_supported() {
  uint32_t instance_layer_count = 0;
  vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
  VkLayerProperties *layer_properties = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * instance_layer_count);
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

bool vk_create_instance(vulkan_context *context) {
  // TODO: We *always* declare KHRONOS_validation,
  // but whether it gets passed to the VkInstance is
  // determined lower. This feels weird, since if we
  // want any other layers, then they will all be 
  // "vetoed" just if KHRONOS_validation is not supported.
  const char *layers[] = {
    VK_LAYER_KHRONOS_VALIDATION_NAME 
  };

  uint32_t extension_count = 0;
  const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);

  SDL_Log("[INFO] Instance Extensions:\n");
  for (uint32_t i = 0; i < extension_count; ++i) {
    const char* const ext = extensions[i];
    SDL_Log("[INFO]\t%s\n", ext);
  }

  VkInstanceCreateInfo vk_create_info = {0};
  vk_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  vk_create_info.enabledExtensionCount = extension_count;
  vk_create_info.ppEnabledExtensionNames = extensions;

  if (is_khronos_validation_supported()) {
    vk_create_info.enabledLayerCount = 1;
    vk_create_info.ppEnabledLayerNames = layers;
  } else {
    SDL_Log("[WARNING] Khronos validation is not supported by your system. Debug information may be limited or non-existant.\n");
  }

  return vkCreateInstance(&vk_create_info, NULL, &context->instance) == VK_SUCCESS;
}

bool vk_create_surface(window_context *win, vulkan_context *vk) {
  return SDL_Vulkan_CreateSurface(
    win->window, 
    vk->instance, 
    NULL, 
    &vk->surface);
}

void vk_choose_physical_device(vulkan_context *vk) {
  uint32_t gpu_dev_count = 0;
  vkEnumeratePhysicalDevices(vk->instance, &gpu_dev_count, NULL);
  if (gpu_dev_count == 0) {
    SDL_Log("[ERROR] Failed to find physical GPUs with Vulkan support.\n");
    // Maybe need engine_shutdown();
    exit(1);
  }
  VkPhysicalDevice *devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_dev_count);
  if (!devices) {
    SDL_Log("[ERROR] Tried to allocate memory to enumerate physical devices, but ran out of memory.\n");
    exit(1);
  }
  vkEnumeratePhysicalDevices(vk->instance, &gpu_dev_count, devices);

  vk->phys_device = VK_NULL_HANDLE;
  for (uint32_t i = 0; i < gpu_dev_count; ++i) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(devices[i], &properties);
    SDL_Log("[INFO] Found GPU Device: %s\n", properties.deviceName);
    const char* type;
    switch (properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   type = "Discrete GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: type = "Integrated GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    type = "Virtual GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:            type = "CPU"; break;
        default:                                     type = "Other"; break;
    }
    SDL_Log("\tType: %s\n", type);
    SDL_Log("\tAPI Version: %u.%u.%u\n", 
           VK_API_VERSION_MAJOR(properties.apiVersion),
           VK_API_VERSION_MINOR(properties.apiVersion),
           VK_API_VERSION_PATCH(properties.apiVersion));

    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      vk->phys_device = devices[i];

      SDL_Log("[INFO] '%s' chosen as target GPU.\n", properties.deviceName);
      break;
    }
  }

  if (vk->phys_device == VK_NULL_HANDLE) {
    // TODO: Is it really a good idea to just fall back on device 0?
    vk->phys_device = devices[0];
  }

  free(devices);
}

void vk_query_queue_families(vulkan_context *vk) {
  vkGetPhysicalDeviceQueueFamilyProperties(vk->phys_device, &vk->queue_family_count, NULL);
 
  vk->queue_families = 
    (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * vk->queue_family_count);
  if (!vk->queue_families) {
    SDL_Log("[ERROR] Tried to allocate memory to enumerate physical devices, but ran out of memory.\n");
    exit(1);
  }
  vkGetPhysicalDeviceQueueFamilyProperties(vk->phys_device, &vk->queue_family_count, vk->queue_families);
}

int vk_find_graphics_queue_family_idx(vulkan_context *vk) {
  for (uint32_t i = 0; i < vk->queue_family_count; ++i) {
    if (vk->queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      return i;
    }
  }
  return -1;
}

VkDeviceQueueCreateInfo vk_init_queue_create_info(int queue_family_idx) {
  float queue_priority = 1.0f;
  VkDeviceQueueCreateInfo queue_create_info = {0};
  queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = queue_family_idx;
  queue_create_info.queueCount = 1;
  queue_create_info.pQueuePriorities = &queue_priority;
  return queue_create_info;
}

void vk_create_device(vulkan_context *vk, VkDeviceQueueCreateInfo queue_create_info) {
  const char *device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  VkDeviceCreateInfo dev_create_info = {0};
  dev_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  dev_create_info.queueCreateInfoCount = 1;
  dev_create_info.pQueueCreateInfos = &queue_create_info;

  dev_create_info.enabledExtensionCount = 1;
  dev_create_info.ppEnabledExtensionNames = device_extensions;

  if (vkCreateDevice(vk->phys_device, &dev_create_info, NULL, &vk->device) != VK_SUCCESS) {
    SDL_Log("[ERROR] Failed to initialize logical device.\n");
    exit(1);
  }
}

VkBool32 vk_check_queue_family_supports_surface(vulkan_context *vk, int queue_family_idx) {
  VkBool32 phys_surf_supported;
  vkGetPhysicalDeviceSurfaceSupportKHR(
    vk->phys_device,
    queue_family_idx,
    vk->surface,
    &phys_surf_supported
      );
  return phys_surf_supported;
}

VkSurfaceFormatKHR __vk_choose_swapchain_format(vulkan_context *vk) {
  for (uint32_t i = 0; i < vk->swapchain_support.format_count; ++i) {
    VkSurfaceFormatKHR format = vk->swapchain_support.formats[i];
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        return format;
  }

  // TODO: Again, I don't know if returning the *first* one is correct.
  // Maybe it is worth searching for one that at least matches color space or something,
  // even if the format isn't perfect.
  return vk->swapchain_support.formats[0];
}

VkPresentModeKHR __vk_choose_present_mode(vulkan_context *vk) {
  for (uint32_t i = 0; i < vk->swapchain_support.present_mode_count; ++i) {
    if (vk->swapchain_support.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      return vk->swapchain_support.present_modes[i];
    }
  }

  // NOTE: This is *guaranteed* by the Vulkan spec to always be
  // available, so not even needed to search in the list of 
  // supported present modes.
  // See: https://docs.vulkan.org/refpages/latest/refpages/source/VkPresentModeKHR.html
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D __vk_choose_swap_extent(window_context *win, VkSurfaceCapabilitiesKHR capabilities) {
  // NOTE: 0xFFFFFFFF implies that we basically have to choose an extent
  // rather than allow vkGetPhysicalDeviceSurfaceCapabilitiesKHR to determine it
  // for us.
  if (capabilities.currentExtent.width != 0xFFFFFFFF) {
    return capabilities.currentExtent;
  } else {
    int w, h;
    SDL_GetWindowSizeInPixels(win->window, &w, &h);

    VkExtent2D actual_extent = {
      (uint32_t) w,
      (uint32_t) h
    };

    actual_extent.width = SDL_clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height = SDL_clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actual_extent;
  }
}

void __vk_get_swapchain_images(vulkan_context *vk) {
  vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &vk->swapchain_image_count, NULL);
  vk->swapchain_images = (VkImage*)malloc(sizeof(VkImage) * vk->swapchain_image_count);
  fail_check(vk->swapchain_images != NULL, "[ERROR] Tried to allocate on the heap, but ran out of memory.\n");
  vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &vk->swapchain_image_count, vk->swapchain_images);
}

void __vk_create_image_views(vulkan_context *vk) {
  vk->swapchain_image_views = (VkImageView*)malloc(sizeof(VkImageView) * vk->swapchain_image_count);
  fail_check(vk->swapchain_image_views != NULL, "[ERROR] Tried to allocate on the heap, but ran out of memory.\n");

  for (uint32_t i = 0; i < vk->swapchain_image_count; ++i) {
    VkImageViewCreateInfo create_view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = vk->swapchain_images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = vk->chosen_format.format,
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

    VkResult res = vkCreateImageView(vk->device, &create_view_info, NULL, &vk->swapchain_image_views[i]);
    fail_check(res == VK_SUCCESS, "[ERROR] Failed to create image view.\n");
  }
}

void vk_create_swapchain(window_context *win, vulkan_context *vk) {
  // TODO: For every Vulkan function that returns a VkResult, we should really check if it succeeded or not.
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->phys_device, vk->surface, &vk->swapchain_support.capabilities);

  vkGetPhysicalDeviceSurfaceFormatsKHR(vk->phys_device, vk->surface, &vk->swapchain_support.format_count, NULL);
  vk->swapchain_support.formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * vk->swapchain_support.format_count);
  fail_check(vk->swapchain_support.formats != NULL, "[ERROR] Tried to allocate on the heap, but ran out of memory.\n");
  vkGetPhysicalDeviceSurfaceFormatsKHR(vk->phys_device, vk->surface, &vk->swapchain_support.format_count, vk->swapchain_support.formats);
  
  VkSurfaceFormatKHR chosen_format = __vk_choose_swapchain_format(vk);
  vk->chosen_format = chosen_format;

  vkGetPhysicalDeviceSurfacePresentModesKHR(vk->phys_device, vk->surface, &vk->swapchain_support.present_mode_count, NULL);
  vk->swapchain_support.present_modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * vk->swapchain_support.present_mode_count);
  fail_check(vk->swapchain_support.present_modes != NULL, "[ERROR] Tried to allocate on the heap, but ran out of memory.\n");
  vkGetPhysicalDeviceSurfacePresentModesKHR(vk->phys_device, vk->surface, &vk->swapchain_support.present_mode_count, vk->swapchain_support.present_modes);

  VkPresentModeKHR chosen_present_mode = __vk_choose_present_mode(vk);
  VkExtent2D chosen_extent = __vk_choose_swap_extent(win, vk->swapchain_support.capabilities);
  vk->chosen_extent = chosen_extent;

  // We add 1, as we want minimum of triple-buffering.
  uint32_t image_count = vk->swapchain_support.capabilities.minImageCount + 1;
  if (vk->swapchain_support.capabilities.maxImageCount > 0 &&
      image_count > vk->swapchain_support.capabilities.maxImageCount) {
        image_count = vk->swapchain_support.capabilities.maxImageCount;
      }

  VkSwapchainCreateInfoKHR swapchain_create_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = vk->surface,
    .minImageCount = image_count,
    .imageFormat = chosen_format.format,
    .imageColorSpace = chosen_format.colorSpace,
    .imageExtent = chosen_extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .preTransform = vk->swapchain_support.capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = chosen_present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE
  };

  // Setting these separately, since these only apply while
  // we have ONE queue (family).
  swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkResult res = vkCreateSwapchainKHR(vk->device, &swapchain_create_info, NULL, &vk->swapchain);
  fail_check(res == VK_SUCCESS, "[ERROR] Failed to create swapchain.\n");

  free_and_set_null(vk->swapchain_support.formats);
  free_and_set_null(vk->swapchain_support.present_modes);

  __vk_get_swapchain_images(vk);
  __vk_create_image_views(vk);
}

void vk_create_render_pass(vulkan_context *vk) {
  // NOTE: Attachment Descriptions/Render Passes were *superseceded* by Vulkan 1.2
  // but we still use them for now.
  VkAttachmentDescription color_attachment_description = {
    .format = vk->chosen_format.format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  };

  VkAttachmentReference color_attachment_reference = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  VkSubpassDescription subpass = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment_reference
  };

  // NOTE: Apparently this is used to stop the GPU outputting to the screen whilst
  // it's already being outputted to? I don't fully understand this yet, so more
  // research needed.
  VkSubpassDependency subpass_dependency = {
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
    .pAttachments = &color_attachment_description,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 1,
    .pDependencies = &subpass_dependency
  };

  VkResult res = vkCreateRenderPass(vk->device, &render_pass_create_info, NULL, &vk->render_pass);
  fail_check(res == VK_SUCCESS, "[ERROR] Failed to create render pass.\n");
}

void __vk_create_frame_buffer(vulkan_context *vk, uint32_t idx) {
  VkImageView attachments[] = {
    vk->swapchain_image_views[idx]
  };

  VkFramebufferCreateInfo frame_buffer_create_info = {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .renderPass = vk->render_pass,
    .attachmentCount = 1,
    .pAttachments = attachments,
    .width = vk->chosen_extent.width,
    .height = vk->chosen_extent.height,
    .layers = 1
  };

  VkResult res = vkCreateFramebuffer(vk->device, &frame_buffer_create_info, NULL, &vk->swapchain_frame_buffers[idx]);
  fail_check(res == VK_SUCCESS, "[ERROR] Failed to create frame buffer.\n");
}

void vk_create_frame_buffers(vulkan_context *vk) {
  vk->swapchain_frame_buffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * vk->swapchain_image_count);
  fail_check(vk->swapchain_frame_buffers != NULL, "[ERROR] Tried to allocate on the heap, but ran out of memory.\n");

  for (uint32_t i = 0; i < vk->swapchain_image_count; ++i) {
    __vk_create_frame_buffer(vk, i);
  }
}

void vk_create_pipeline_layout(vulkan_context *vk) {
  // TODO: This will need to change when we actually take
  // inputs/uniforms into our shaders.
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 0,
    .pSetLayouts = NULL,
    .pushConstantRangeCount = 0,
    .pPushConstantRanges = NULL
  };

  VkResult res = vkCreatePipelineLayout(vk->device, &pipeline_layout_create_info, NULL, &vk->pipeline_layout);
  fail_check(res == VK_SUCCESS, "[ERROR] Failed to create pipeline layout.\n");
}

void vk_create_shader_module(vulkan_context *vk, uint32_t *code, size_t size, VkShaderModule *module) {
  VkShaderModuleCreateInfo shader_module_create_info = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = size,
    .pCode = code
  };

  VkResult res = vkCreateShaderModule(vk->device, &shader_module_create_info, NULL, module);
  fail_check(res == VK_SUCCESS, "[ERROR] Failed to create shader module.\n");
}

void vk_create_pipeline(vulkan_context *vk) {
  VkPipelineShaderStageCreateInfo shader_stage_vert_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = vk->vert,
    .pName = "MainVS"
  };

  VkPipelineShaderStageCreateInfo shader_stage_frag_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = vk->frag,
    .pName = "MainPS"
  };

  VkPipelineShaderStageCreateInfo shader_stages[] = {
    shader_stage_vert_create_info,
    shader_stage_frag_create_info
  };

  // This is basically empty right now as we have hardcoded
  // triangle vertices in the shader.
  VkPipelineVertexInputStateCreateInfo vertex_input_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 0,
    .pVertexBindingDescriptions = NULL,
    .vertexAttributeDescriptionCount = 0,
    .pVertexAttributeDescriptions = NULL
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE
  };

  VkViewport viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = (float)vk->chosen_extent.width,
    .height = (float)vk->chosen_extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f
  };

  VkRect2D scissor = {
    .offset = {0, 0},
    .extent = vk->chosen_extent
  };

  VkPipelineViewportStateCreateInfo viewport_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor
  };

  VkPipelineRasterizationStateCreateInfo rasterizer_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .lineWidth = 1.0f,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable = VK_FALSE
  };

  VkPipelineMultisampleStateCreateInfo multisampling_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .sampleShadingEnable = VK_FALSE,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
  };

  VkPipelineColorBlendAttachmentState color_blend_attachment = {
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                    | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    .blendEnable = VK_FALSE
  };

  VkPipelineColorBlendStateCreateInfo color_blending_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment
  };

  VkGraphicsPipelineCreateInfo pipeline_create_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = 2,
    .pStages = shader_stages,
    .pVertexInputState = &vertex_input_info,
    .pInputAssemblyState = &input_assembly_info,
    .pViewportState = &viewport_info,
    .pRasterizationState = &rasterizer_info,
    .pMultisampleState = &multisampling_info,
    .pColorBlendState = &color_blending_info,
    .layout = vk->pipeline_layout,
    .renderPass = vk->render_pass,
    .subpass = 0
  };

  VkResult res = vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &vk->pipeline);
  fail_check(res == VK_SUCCESS, "[ERROR] Failed to create graphics pipeline.\n");
}

void vk_create_command_pool(vulkan_context *vk, int queue_family_idx) {
  VkCommandPoolCreateInfo pool_create_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = queue_family_idx
  };

  VkResult res = vkCreateCommandPool(vk->device, &pool_create_info, NULL, &vk->command_pool);
  fail_check(res == VK_SUCCESS, "[ERROR] Failed to create command pool.\n");
}

void vk_allocate_command_buffer(vulkan_context *vk) {
  VkCommandBufferAllocateInfo buffer_create_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = vk->command_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1
  };

  VkResult res = vkAllocateCommandBuffers(vk->device, &buffer_create_info, &vk->command_buffer);
  fail_check(res == VK_SUCCESS, "[ERROR] Failed to allocate command buffer.\n");
}