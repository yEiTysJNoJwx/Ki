#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.h"
#include "gfx/window.h"

#include <exception>
#include <iostream>
#include <string>

int main()
{
  std::string build_type;

#ifndef NDEBUG
  build_type = "Debug";
#else
  build_type = "Release";
#endif

  std::cout << "Build type: " << build_type << "\n";

  try {
    ki::Application app;
    app.Run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    ki::gfx::Window::TerminateGlfw();
    return 1;
  }
}
