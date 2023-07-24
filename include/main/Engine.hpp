#pragma once

#include <memory>

#include "Window.hpp"
#include "Renderer.hpp"
#include "ECS.hpp"

class Engine
{
public:
  ~Engine();

  Manager entitiesManager; // TODO: Make this private when tests end.
  Entity &ent3DTest;
private:
  Entity &camera;
  inline static std::shared_ptr<Engine> instance;
  std::unique_ptr<Window> window;
  std::unique_ptr<Renderer> renderer;

  Engine();
  void processMemUsage(double& vm_usage, double& resident_set);
  void printOS();
  void printDevKeyBinds();
  void toggleGraphicsSettings();
  void mainLoop();

  // TODO: Put this in an Utils file.
  /**
   * @brief Checks if a weak pointer hasn't been initialized.
   * 
   * @tparam T 
   * @param weak 
   * @return true 
   * @return false 
   */
  template <typename T>
  static bool isUninitialized(std::weak_ptr<T> const& weak) {
    using wt = std::weak_ptr<T>;
    return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
  }

public:
  void init();
  void run();
  void attachWindow(std::unique_ptr<Window> window);
  void attachRenderer(std::unique_ptr<Renderer> renderer);

  static std::shared_ptr<Engine> get();

  // Getters and Setters

  const std::unique_ptr<Window> &getWindow() const;
  const std::unique_ptr<Renderer> &getRenderer() const;
  const Entity &getCamera() const;
};