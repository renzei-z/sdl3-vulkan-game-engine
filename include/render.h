#ifndef RENDER_H_
#define RENDER_H_

#include <vulkan/vulkan.h>

#include <vulkan/vulkan_core.h>
#include <window.h>

#define VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"

typedef struct swapchain_support_info_t {
  VkSurfaceCapabilitiesKHR capabilities;
  uint32_t format_count;
  VkSurfaceFormatKHR *formats;
  uint32_t present_mode_count;
  VkPresentModeKHR *present_modes;
} swapchain_support_info;

typedef struct vulkan_context_t {
  VkInstance instance;
  VkSurfaceKHR surface;

  VkPhysicalDevice phys_device;
  VkDevice device;

  uint32_t queue_family_count;
  VkQueueFamilyProperties *queue_families;

  swapchain_support_info swapchain_support;
  uint32_t swapchain_image_count;
  VkImageView *swapchain_image_views;
  VkImage *swapchain_images;
  VkFramebuffer *swapchain_frame_buffers;
  VkSwapchainKHR swapchain;

  VkSurfaceFormatKHR chosen_format;
  VkExtent2D chosen_extent;

  VkRenderPass render_pass;

  VkPipelineLayout pipeline_layout;

  // TODO: We need a better way to store multiple shaders.
  VkShaderModule vert, frag;

  VkPipeline pipeline;

  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;
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
void vk_create_swapchain(window_context *win, vulkan_context *vk);
void vk_create_render_pass(vulkan_context *vk);
void vk_create_frame_buffers(vulkan_context *vk);
void vk_create_pipeline_layout(vulkan_context *vk);
void vk_create_shader_module(vulkan_context *vk, uint32_t *code, size_t size, VkShaderModule *module);
void vk_create_pipeline(vulkan_context *vk);
void vk_create_command_pool(vulkan_context *vk, int queue_family_idx);
void vk_allocate_command_buffer(vulkan_context *vk);

#endif // RENDER_H_
