#include "Engine.hpp"
#include "KeyListener.hpp"
#include "AssetPool.hpp"

#include "Transform.hpp"
#include "PerspectiveCamera.hpp"
#include "ModelRenderer.hpp"
#include "TextureRenderer.hpp"

#ifdef unix
#include <iostream>
#include <fstream>
#include <unistd.h>
#elif _WIN32
// TODO: Windows imports for RAM calculations
#endif

#ifdef IMGUI_ENABLED
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#endif

Engine::Engine() : camera(entitiesManager.addEntity())
{
  
}

Engine::~Engine()
{
  
}

void Engine::init()
{
  this->printOS();
  camera.addComponent<PerspectiveCamera>();

  this->renderer->init();
  AssetPool::addTexture(this->renderer->getDevice(), "img_tex", "assets/textures/viking_room.png");
  AssetPool::addTexture(this->renderer->getDevice(), "img_tex2", "assets/textures/img.jpg");
  AssetPool::addShader(this->renderer->getDevice(), "texture", "shaders/texture_fragment_shader.spv", "shaders/texture_vertex_shader.spv");
  AssetPool::addModel("model", "assets/models/viking_room.obj");

  for (int i = 0; i < 20; i++) {
    Entity &e(entitiesManager.addEntity());
    e.addComponent<Transform>(glm::vec3(0, i * 2.0f, 0));
    e.addComponent<ModelRenderer>(AssetPool::getModel("model"));
    if (i == 3)
      e.addComponent<TextureRenderer>(AssetPool::getTexture("img_tex2"));
    else
      e.addComponent<TextureRenderer>(AssetPool::getTexture("img_tex"));

    this->renderer->addEntity(e);
  }

  this->renderer->initRendering();
  this->printDevKeyBinds();
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

void Engine::toggleGraphicsSettings()
{
  // Toggle mipmap setting
  if (KeyListener::isBindDown(GLFW_KEY_LEFT_SHIFT, GLFW_KEY_F1)) {
    this->renderer->mipmapSetting++;
    std::cout << "Mipmap setting changed to '" << this->renderer->mipmapSetting << "'.\n";
    this->renderer->restart();
  }
  // Iterate MSAA settings.
  else if (KeyListener::isBindDown(GLFW_KEY_LEFT_SHIFT, GLFW_KEY_F2)) {
    this->renderer->msaaSetting++;
    std::cout << "MSAA setting changed to '" << this->renderer->msaaSetting << "'.\n";
    this->renderer->restart();
  }
  else if (KeyListener::isBindDown(GLFW_KEY_LEFT_SHIFT, GLFW_KEY_F3)) {
    this->renderer->sampleShading = !this->renderer->sampleShading;
    std::cout << "Sample shading setting changed to '" << this->renderer->sampleShading << "'.\n";
    this->renderer->restart();
  }
}

void Engine::printDevKeyBinds()
{
  std::cout << " ->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->\n";
  std::cout << "|  Useful developer keybindings:\n";
  std::cout << "|    SHIFT + F1 -> Toggles Mipmap setting (DISABLED, LINEAR, NEAREST).\n";
  std::cout << "|    SHIFT + F2 -> Iterates MSAA settings (DISABLED, MSAA2X, MSAA4X,\n";
  std::cout << "|                  MSAA8X, MSAA16X, MSAA32X, MSAA64X).\n";
  std::cout << "|    SHIFT + F3 -> Toggles Sample Shading setting (False, True).\n";
  std::cout << " ->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->\n";
}

void Engine::mainLoop()
{
  float lastTime = glfwGetTime();
  float accumulator = 0.0f;
  const unsigned short MAX_FPS = 60;
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

    if (delta < MAX_FPS_PER_SEC) {
      delta = MAX_FPS_PER_SEC;
    }
    lastTime = currentTime;

    // Controls frame rate.
    if (accumulator > MAX_FPS_PER_SEC) {
      KeyListener::update();
      glfwPollEvents();

      this->entitiesManager.update(delta);

      this->toggleGraphicsSettings();

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

const Entity &Engine::getCamera() const
{
  return this->camera;
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