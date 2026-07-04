#include "vulkan_context.h"

#include "vulkan_support.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <map>
#include <functional>
#include <utility>
#include <set>

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
  
  if (glfwCreateWindowSurface(instance_, window, nullptr, &surface))
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

    if (!features.geometryShader || !queue_families.RequiredQueueFamiliesAvailable())
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

  device_create_info.enabledExtensionCount = 0;

  logical_device_ = physical_device_.createDevice(device_create_info);

  graphics_queue_ = logical_device_.getQueue(queue_families.graphics_family_.value(), 0);
  present_queue_ = logical_device_.getQueue(queue_families.present_family_.value(), 0);
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

  std::vector<const char*> instance_extensions (instance_extensions_required, instance_extensions_required + instance_extension_count);

#ifndef NDEBUG
  instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

  return instance_extensions;
}
