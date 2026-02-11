#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <render.h>
#include <window.h>

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