#ifndef VK_CONTEXT_H_
#define VK_CONTEXT_H_

#include <core/cpp_header_guard.h>

HEADER_BEGIN

#include <core/window.h>
#include <renderer/vertex.h>

#include <vulkan/vulkan_core.h>

#include <vk_mem_alloc.h>

#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_VERTICES 10000

typedef struct swapchain_support_details_t {
  VkSurfaceCapabilitiesKHR caps;
  uint32_t format_count;
  VkSurfaceFormatKHR *formats;
  uint32_t present_mode_count;
  VkPresentModeKHR *present_modes;
} swapchain_support_details;

typedef struct vk_context_t {
  window win;

  VkInstance instance;
  VkSurfaceKHR surface;

  VkPhysicalDevice physical_device;
  VkDevice device;

  uint32_t queue_family_count;
  VkQueueFamilyProperties *queue_family_properties;

  uint32_t graphics_family_idx;
  uint32_t compute_family_idx;
  uint32_t transfer_family_idx;

  VkQueue graphics_queue;
  VkQueue compute_queue;
  VkQueue transfer_queue;
  VkCommandPool graphics_pool;
  // VkCommandPool compute_pool;
  // VkCommandPool transfer_pool;
  VkCommandBuffer *command_buffers;

  VkSwapchainKHR swapchain;
  swapchain_support_details swapchain_support;
  uint32_t image_count;
  VkImage *swapchain_images;
  VkImageView *swapchain_image_views;
  VkFormat swapchain_format;
  VkExtent2D swapchain_extent;

  VkRenderPass render_pass;
  VkFramebuffer *framebuffers;
  bool framebuffer_resized;

  VkSemaphore *image_available_semaphores;
  VkSemaphore *render_finished_semaphores;
  VkFence *in_flight_fences;
  uint8_t current_frame;

  VkPipelineLayout pipeline_layout;
  VkPipeline tri_pipeline;

  VmaAllocator allocator;
  VkBuffer buffer;
  VmaAllocation allocation;
} vk_context;

typedef struct vk_pipeline_config_t {
  VkPipelineInputAssemblyStateCreateInfo input_assembly;
  VkPipelineRasterizationStateCreateInfo rasterizer;
  VkPipelineMultisampleStateCreateInfo multisampling;
  VkPipelineColorBlendAttachmentState color_blend_attachment;
  VkPipelineLayout layout;
  VkRenderPass render_pass;
} vk_pipeline_config;

vk_pipeline_config vk_default_pipeline_config();

void vk_context_init(vk_context *ctx, const char *title, int width, int height);

VkPipeline vk_pipeline_build(vk_context *ctx, const char *vs_path, const char *fs_path, vk_pipeline_config *config);

void vk_draw_frame(vk_context *ctx, uint32_t vertex_count);

void vk_context_shutdown(vk_context *ctx);

HEADER_END

#endif // VK_CONTEXT_H_
