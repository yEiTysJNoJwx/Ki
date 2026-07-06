#ifndef KI_GFX_VULKAN_CONTEXT_H_
#define KI_GFX_VULKAN_CONTEXT_H_

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "window.h"

#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace ki::gfx
{

class Context
{
  private:
    vk::Instance instance_ = nullptr;
    vk::PhysicalDevice physical_device_ = nullptr;
    vk::Device logical_device_ = nullptr;

    vk::Queue graphics_queue_ = nullptr;
    vk::Queue present_queue_ = nullptr;

    vk::SurfaceKHR surface_ = nullptr;

    vk::SwapchainKHR swap_chain_ = nullptr;

    std::string app_name_;
    uint32_t app_version_;

    std::string engine_name_;
    uint32_t engine_version_;

    static inline std::vector<const char*> validation_layers = {
      "VK_LAYER_KHRONOS_validation"
    };

    static inline std::vector<const char*> device_extensions = {
      vk::KHRSwapchainExtensionName
    };
  public:
    Context(const char* app_name, const char* engine_name = "No Engine",
	uint32_t app_version = VK_MAKE_VERSION(1, 0, 0),
	uint32_t engine_version = VK_MAKE_VERSION(1, 0, 0));
    Context() = default;

    void CreateVkInstance();
    void CreateSurface(GLFWwindow* window);
    void LocatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapChain(const Window& window);

    void Cleanup();
    void DestroyInstance();

    vk::Instance& instance() { return instance_; }
};

std::vector<const char*> GetRequiredInstanceExtensions();

};

#endif
