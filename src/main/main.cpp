#include "Engine.hpp"

int main()
{
  std::shared_ptr<Engine> eng = Engine::get();

  // Configure engine
  std::unique_ptr<Window> window = std::make_unique<Window>("Vulkan", 800, 600, false);
  eng->attachWindow(std::move(window));

  std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>();

  // TODO: Configuration file that can save settings like this.
  renderer->mipmapSetting = Renderer::MipmapSetting::DISABLED;
  renderer->msaaSetting   = Renderer::MsaaSetting::MSAA8X;

  eng->attachRenderer(std::move(renderer));

  // Init
  eng->init();

  // Launch
  try {
    eng->run();
  }
  catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
