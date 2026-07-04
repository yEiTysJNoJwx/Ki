#include "application.h"

using namespace ki;

void Application::Run()
{
  Init();
  Mainloop();
  Cleanup();
}

void Application::Init()
{
  gfx::Window::InitGlfw();

  std::string app_name (kApplicationName);

  context_ = gfx::Context(app_name.c_str());
  window_ = gfx::Window(kWindowWidth, kWindowHeight, app_name.c_str());

  context_.CreateVkInstance();

  context_.CreateSurface(window_.window());  

  context_.LocatePhysicalDevice();
  context_.CreateLogicalDevice();
}

void Application::Mainloop()
{
  while (!window_.ShouldClose()) {
    glfwPollEvents();
  }
}

void Application::Cleanup()
{
  context_.Cleanup();

  window_.Destroy();
  gfx::Window::TerminateGlfw();
}
