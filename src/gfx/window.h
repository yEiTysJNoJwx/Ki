#ifndef KI_GFX_WINDOW_H_
#define KI_GFX_WINDOW_H_

#include "vulkan_context.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <string>
#include <memory>

namespace ki::gfx
{

class Window
{
  private:
    GLFWwindow* window_ = nullptr;
  
    uint32_t width_;
    uint32_t height_;

    std::string name_;

  public:
    Window(uint32_t width, uint32_t height, const char* name);
    Window() = default;

    GLFWwindow* window() const { return window_; }

    void Destroy();

    bool ShouldClose();

    static void InitGlfw();
    static void PollEvents();
    static void TerminateGlfw();
};

};

#endif
