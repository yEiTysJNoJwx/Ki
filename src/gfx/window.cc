#include "window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

using namespace ki::gfx;

Window::Window(uint32_t width, uint32_t height, const char* name)
: width_(width), height_(height), name_(name)
{
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window_ = glfwCreateWindow(width_, height_, name, nullptr, nullptr);

  if (!window_)
    throw std::runtime_error("Failed to create GLFWwindow.");
}

void Window::InitGlfw()
{
  if (!glfwInit())
    throw std::runtime_error("Failed to load GLFW.");
}

void Window::PollEvents() { glfwPollEvents(); }
void Window::TerminateGlfw() { glfwTerminate(); }

bool Window::ShouldClose()
{
  return glfwWindowShouldClose(window_);
}

void Window::Destroy()
{
  glfwDestroyWindow(window_);
}
