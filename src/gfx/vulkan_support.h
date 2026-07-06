#ifndef KI_GFX_VULKAN_SUPPORT_H_
#define KI_GFX_VULKAN_SUPPORT_H_

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "window.h"

#include <optional>
#include <cstdint>
#include <vector>

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

struct SwapChainSupport
{
  vk::SurfaceCapabilitiesKHR capabilities_;
  std::vector<vk::SurfaceFormatKHR> formats_;
  std::vector<vk::PresentModeKHR> present_modes_;
};

void ExtensionSupported(const char* extension_name);
bool DeviceExtensionSupported(const vk::PhysicalDevice& physical_device, const char* extension_name);

void ValidationLayerSupported(const char* layer_name);

QueueFamilies GetQueueFamiliesAvailable(const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface);

SwapChainSupport QuerySwapChainSupport(const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface);

vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> formats);
vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> present_modes);
vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window); 

};

#endif
