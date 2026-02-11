#ifndef VK_SETUP_H_
#define VK_SETUP_H_

#include <vulkan/vulkan.h>

#include <window.h>

#define VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"

typedef struct vulkan_context_t {
  VkInstance instance;
  VkSurfaceKHR surface;

  VkPhysicalDevice phys_device;
  VkDevice device;

  uint32_t queue_family_count;
  VkQueueFamilyProperties *queue_families;
} vulkan_context;

bool is_khronos_validation_supported();
bool vk_create_instance(vulkan_context *context);
bool vk_create_surface(window_context *win, vulkan_context *vk);
void vk_choose_physical_device(vulkan_context *vk);
void vk_query_queue_families(vulkan_context *vk);
int vk_find_graphics_queue_family_idx(vulkan_context *vk);
// TODO: Make it possible to create device with multiple queues.
VkDeviceQueueCreateInfo vk_init_queue_create_info(int queue_family_idx);
void vk_create_device(vulkan_context *vk, VkDeviceQueueCreateInfo queue_create_info);
VkBool32 vk_check_queue_family_supports_surface(vulkan_context *vk, int queue_family_idx);

#endif // VK_SETUP_H_
