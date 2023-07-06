#include "Engine.hpp"
#include "KeyListener.hpp"

Engine::Engine()
{
  
}

Engine::~Engine()
{
  
}

void Engine::init()
{
  this->renderer->init();
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