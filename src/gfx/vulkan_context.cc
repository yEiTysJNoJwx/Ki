#include "vulkan_context.h"

#include "vulkan_support.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <map>
#include <functional>
#include <utility>
#include <set>
#include <algorithm>

using namespace ki::gfx;

Context::Context(const char* app_name, const char* engine_name, uint32_t app_version, uint32_t engine_version)
: app_name_(app_name), app_version_(app_version), engine_name_(engine_name), engine_version_(engine_version)
{}

void Context::CreateVkInstance()
{
  vk::ApplicationInfo app_info = {};
  app_info.pApplicationName = app_name_.c_str();
  app_info.applicationVersion = app_version_;
  app_info.pEngineName = engine_name_.c_str();
  app_info.engineVersion = engine_version_;
  app_info.apiVersion = vk::ApiVersion14;

  vk::InstanceCreateInfo instance_create_info {};
  instance_create_info.pApplicationInfo = &app_info;
  
  auto instance_extensions = gfx::GetRequiredInstanceExtensions();

  for (const char* extension : instance_extensions)
    gfx::ExtensionSupported(extension);

  instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
  instance_create_info.ppEnabledExtensionNames = instance_extensions.data();

  instance_create_info.enabledLayerCount = 0;

#ifndef NDEBUG
  for (const char* validation_layer : validation_layers)
    gfx::ValidationLayerSupported(validation_layer);

  instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
  instance_create_info.ppEnabledLayerNames = validation_layers.data();
#endif

  instance_ = vk::createInstance(instance_create_info);
}

void Context::CreateSurface(GLFWwindow* window)
{
  VkSurfaceKHR surface = nullptr;
  
  if (glfwCreateWindowSurface(instance_, window, nullptr, &surface) != VK_SUCCESS)
    throw std::runtime_error("Failed to create window surface.");

  surface_ = vk::SurfaceKHR(surface);
}

void Context::LocatePhysicalDevice()
{
  auto physical_devices = instance_.enumeratePhysicalDevices();

  if (physical_devices.empty())
    throw std::runtime_error("No physical devices with Vulkan support.");

  std::multimap<uint32_t, vk::PhysicalDevice, std::greater<uint32_t>> candidates;

  for (const auto& physical_device : physical_devices) {
    auto props = physical_device.getProperties();
    auto features = physical_device.getFeatures();

    struct QueueFamilies queue_families = gfx::GetQueueFamiliesAvailable(physical_device, surface_);

    uint32_t score = 0;

    if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
      score += 10000;
    } else if (props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
      score += 5000;
    } else if (props.deviceType == vk::PhysicalDeviceType::eVirtualGpu) {
      score += 2500;
    } else if (props.deviceType == vk::PhysicalDeviceType::eCpu) {
      score += 1000;
    }

    score += props.limits.maxImageDimension2D;

    bool supports_extensions = true;

    if (std::ranges::any_of(device_extensions, [physical_device = physical_device](const char* extension)
	  { return !DeviceExtensionSupported(physical_device, extension); }))
      supports_extensions = false;

    bool swap_chain_adequate = false;

    if (supports_extensions) {
      SwapChainSupport swap_chain_support = QuerySwapChainSupport(physical_device, surface_);
      
      swap_chain_adequate = !swap_chain_support.formats_.empty() && !swap_chain_support.present_modes_.empty();
    }

    if (!features.geometryShader || !queue_families.RequiredQueueFamiliesAvailable() || !supports_extensions || !swap_chain_adequate)
      score = 0;

    candidates.insert({score, physical_device});
  }

  if (candidates.begin()->first > 0) {
    physical_device_ = candidates.begin()->second;
  } else {
    throw std::runtime_error("Failed to find a suitable GPU.");
  }
}

void Context::CreateLogicalDevice()
{
  QueueFamilies queue_families = gfx::GetQueueFamiliesAvailable(physical_device_, surface_);

  std::vector<vk::DeviceQueueCreateInfo> device_queue_create_info;

  std::set<uint32_t> unique_queue_families = {
    queue_families.graphics_family_.value(),
    queue_families.present_family_.value()
  };

  float queue_priority = 1.0f;
  for (uint32_t family : unique_queue_families) {
    vk::DeviceQueueCreateInfo queue_create_info {};
    queue_create_info.queueFamilyIndex = family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    
    device_queue_create_info.push_back(queue_create_info);
  }

  vk::PhysicalDeviceFeatures features {};

  vk::DeviceCreateInfo device_create_info {};
  device_create_info.pQueueCreateInfos = device_queue_create_info.data();
  device_create_info.queueCreateInfoCount = static_cast<uint32_t>(device_queue_create_info.size());

  device_create_info.pEnabledFeatures = &features;

  device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
  device_create_info.ppEnabledExtensionNames = device_extensions.data();

  logical_device_ = physical_device_.createDevice(device_create_info);

  graphics_queue_ = logical_device_.getQueue(queue_families.graphics_family_.value(), 0);
  present_queue_ = logical_device_.getQueue(queue_families.present_family_.value(), 0);
}

void Context::CreateSwapChain(const Window& window)
{
  SwapChainSupport swap_chain_details = QuerySwapChainSupport(physical_device_, surface_);

  vk::SurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(swap_chain_details.formats_);
  vk::PresentModeKHR present_mode = ChooseSwapPresentMode(swap_chain_details.present_modes_);
  vk::Extent2D extent = ChooseSwapExtent(swap_chain_details.capabilities_, window);

  uint32_t image_count = swap_chain_details.capabilities_.minImageCount + 1;

  if (swap_chain_details.capabilities_.maxImageCount > 0 && image_count > swap_chain_details.capabilities_.maxImageCount)
    image_count = swap_chain_details.capabilities_.maxImageCount;

  vk::SwapchainCreateInfoKHR swap_chain_create_info {};
  swap_chain_create_info.surface = surface_;

  swap_chain_create_info.minImageCount = image_count;
  swap_chain_create_info.imageFormat = surface_format.format;
  swap_chain_create_info.imageColorSpace = surface_format.colorSpace;
  swap_chain_create_info.imageExtent = extent;
  swap_chain_create_info.imageArrayLayers = 1;
  swap_chain_create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

  QueueFamilies queue_families = GetQueueFamiliesAvailable(physical_device_, surface_);

  uint32_t queue_family_indices[] = {
    queue_families.graphics_family_.value(),
    queue_families.present_family_.value()
  };

  if (queue_families.graphics_family_ != queue_families.present_family_) {
    swap_chain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
    swap_chain_create_info.queueFamilyIndexCount = 2;
    swap_chain_create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    swap_chain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
    swap_chain_create_info.queueFamilyIndexCount = 0;
    swap_chain_create_info.pQueueFamilyIndices = nullptr;
  }

  swap_chain_create_info.preTransform = swap_chain_details.capabilities_.currentTransform;

  swap_chain_create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

  swap_chain_create_info.presentMode = present_mode;
  swap_chain_create_info.clipped = vk::True;

  swap_chain_create_info.oldSwapchain = nullptr;

  swap_chain_ = logical_device_.createSwapchainKHR(swap_chain_create_info);
}

void Context::Cleanup()
{
  logical_device_.waitIdle();

  logical_device_.destroy();
  instance_.destroySurfaceKHR(surface_);
  instance_.destroy();
  }

void Context::DestroyInstance()
{
  instance_.destroy();
}

std::vector<const char*> ki::gfx::GetRequiredInstanceExtensions()
{
  uint32_t instance_extension_count;
  auto instance_extensions_required = glfwGetRequiredInstanceExtensions(&instance_extension_count);

  std::set<const char*> instance_extensions (instance_extensions_required, instance_extensions_required + instance_extension_count);

#ifndef NDEBUG
  instance_extensions.insert(vk::EXTDebugUtilsExtensionName);
#endif

  return std::vector(instance_extensions.begin(), instance_extensions.end());
}
