#include "vulkan_support.h"

#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <cstring>
#include <stdexcept>

using namespace ki::gfx;

void ki::gfx::ExtensionSupported(const char* extension_name)
{
  auto available_extension_props = vk::enumerateInstanceExtensionProperties();
  
  if (std::ranges::none_of(available_extension_props, [extension_name = extension_name](const auto& extension_props)
	{ return strcmp(extension_name, extension_props.extensionName) == 0; }))
    throw std::runtime_error("Extension not supported by Vulkan: " + std::string(extension_name));
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
