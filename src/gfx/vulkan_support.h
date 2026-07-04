#ifndef KI_GFX_VULKAN_SUPPORT_H_
#define KI_GFX_VULKAN_SUPPORT_H_

#include <vulkan/vulkan.hpp>

#include <optional>
#include <cstdint>

namespace ki::gfx
{

struct QueueFamilies
{
  std::optional<uint32_t> graphics_family_;
  std::optional<uint32_t> present_family_;

  bool RequiredQueueFamiliesAvailable() {
    return graphics_family_.has_value() && present_family_.has_value();
  }
};

void ExtensionSupported(const char* extension_name);
void ValidationLayerSupported(const char* layer_name);

QueueFamilies GetQueueFamiliesAvailable(const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface = nullptr);

};

#endif
