#include "vulkan_support.h"

#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <limits>

using namespace ki::gfx;

void ki::gfx::ExtensionSupported(const char* extension_name)
{
  auto available_extension_props = vk::enumerateInstanceExtensionProperties();
  
  if (std::ranges::none_of(available_extension_props, [extension_name = extension_name](const auto& extension_props)
	{ return strcmp(extension_name, extension_props.extensionName) == 0; }))
    throw std::runtime_error("Extension not supported by Vulkan: " + std::string(extension_name));
}

bool ki::gfx::DeviceExtensionSupported(const vk::PhysicalDevice& physical_device, const char* extension_name)
{
  auto available_extension_props = physical_device.enumerateDeviceExtensionProperties();

  if (std::ranges::any_of(available_extension_props, [extension_name = extension_name](const auto& extension_props)
	{ return strcmp(extension_name, extension_props.extensionName) == 0; }))
    return true;

  return false;
}

void ki::gfx::ValidationLayerSupported(const char* layer_name)
{
  auto available_validation_layer_props = vk::enumerateInstanceLayerProperties();

  if (std::ranges::none_of(available_validation_layer_props, [layer_name = layer_name](const auto& layer_props)
	{ return strcmp(layer_name, layer_props.layerName) == 0; }))
    throw std::runtime_error("Validtion layer not supported by Vulkan: " + std::string(layer_name));
}

QueueFamilies ki::gfx::GetQueueFamiliesAvailable(const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface)
{
  QueueFamilies families {};

  auto available_queue_family_props = physical_device.getQueueFamilyProperties();

  for (uint32_t i = 0; i < available_queue_family_props.size(); i++) {
    if (available_queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics) {
      families.graphics_family_ = i;
    }

    if (!surface)
      continue;
    if (!physical_device.getSurfaceSupportKHR(i, surface))
      continue;

    families.present_family_ = i;
  }

  return families;
}

SwapChainSupport ki::gfx::QuerySwapChainSupport(const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface)
{
  SwapChainSupport swap_chain_support {};

  swap_chain_support.capabilities_ = physical_device.getSurfaceCapabilitiesKHR(surface);
  swap_chain_support.formats_ = physical_device.getSurfaceFormatsKHR(surface);
  swap_chain_support.present_modes_ = physical_device.getSurfacePresentModesKHR(surface);

  return swap_chain_support;
}

vk::SurfaceFormatKHR ki::gfx::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> formats)
{
  for (const auto& format : formats) {
    if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
      return format;
  }

  return formats[0];
}

vk::PresentModeKHR ki::gfx::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> present_modes)
{
  for (const auto& present_mode : present_modes) {
    if (present_mode == vk::PresentModeKHR::eMailbox)
      return present_mode;
  }

  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D ki::gfx::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window)
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    window.GetFramebufferSize(&width, &height);

    vk::Extent2D extent = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height)
    };

    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
  }
}
