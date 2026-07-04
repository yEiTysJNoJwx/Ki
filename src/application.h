#ifndef KI_APPLICATION_H_
#define KI_APPLICATION_H_

#include "gfx/window.h"
#include "gfx/vulkan_context.h"

#include <string>

namespace ki
{

class Application
{
  private:
    gfx::Window window_;
    gfx::Context context_;

    static constexpr uint32_t kWindowWidth = 800;
    static constexpr uint32_t kWindowHeight = 600;

    static constexpr std::string_view kApplicationName = "Ki";
  public:
    void Run();

  protected:
    void Init();

    void Mainloop();

    void Cleanup();
};

};

#endif
