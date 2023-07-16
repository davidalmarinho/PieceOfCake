#include "Engine.hpp"
#include "KeyListener.hpp"
#include "AssetPool.hpp"

#ifdef unix
#include <iostream>
#include <fstream>
#include <unistd.h>
#elif _WIN32
// TODO: Windows imports for RAM calculations
#endif

Engine::Engine()
{
  
}

Engine::~Engine()
{
  
}

void Engine::init()
{
  this->printOS();
  this->renderer->init();
}

void Engine::loadAssets()
{
  AssetPool::addShader("texture", "shaders/texture_fragment_shader.spv", "shaders/texture_vertex_shader.spv");
}

void Engine::processMemUsage(double& vm_usage, double& resident_set)
{
#ifdef unix
    vm_usage     = 0.0;
    resident_set = 0.0;

    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
    }

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    vm_usage = vsize / 1024.0;
    resident_set = rss * page_size_kb;
#elif _WIN32
  // TODO: Windows calculation RAM usage
#endif
}

void Engine::printOS()
{
#ifdef unix
  std::cout << "Running on Linux.\n";
#elif _WIN32
  std::cout << "Running on Windows.\n";
#endif
}

void Engine::mainLoop()
{
  float lastTime = glfwGetTime();
  float accumulator = 0.0f;
  const unsigned short MAX_FPS = 6000;
  const float MAX_FPS_PER_SEC = 1.0f / MAX_FPS;

  // Show FPS / Second
  float timer = 0.0f;
  unsigned short fps = 0;
  const uint8_t ONE_SECOND = 1;

  while (!Engine::get()->getWindow()->isReadyToClose()) {
    float currentTime = glfwGetTime();
    float delta = currentTime - lastTime;

    timer += delta;
    accumulator += delta;

    lastTime = currentTime;

    // Controls frame rate.
    if (accumulator > MAX_FPS_PER_SEC) {
      KeyListener::update();
      glfwPollEvents();

      // Call engine logic
      accumulator = 0.0f;
      fps++;
      this->renderer->drawFrame();
    }

    // Get fps per second.
    if (timer > ONE_SECOND) {
      std::cout << "FPS: " << fps << "\n";
      double vm = 0, rss = 0;
      processMemUsage(vm, rss);
      std::cout << "VM: " << vm << " kiB; RSS: " << rss << " kiB \n";
      fps = 0;
      timer = 0.0f;
    }
  }

  vkDeviceWaitIdle(this->renderer->getDevice());
}

void Engine::run()
{
  mainLoop();
  this->renderer.reset();
}

void Engine::attachWindow(std::unique_ptr<Window> window)
{
  this->window = std::move(window);
}

void Engine::attachRenderer(std::unique_ptr<Renderer> renderer)
{
  this->renderer = std::move(renderer);
}

// Getters and Setters

const std::unique_ptr<Window> &Engine::getWindow() const
{
  return window;
}

const std::unique_ptr<Renderer> &Engine::getRenderer() const
{
  return renderer;
}

// Singleton
std::shared_ptr<Engine> Engine::get()
{
  if (instance == nullptr) {
    struct make_shared_enabler : public Engine {};
    instance = std::make_shared<make_shared_enabler>();
  }

  return instance;
}