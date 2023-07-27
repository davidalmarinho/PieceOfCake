#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>

class Component;
class Entity;

using ComponentID = std::size_t;
inline ComponentID getComponentTypeID()
{
  static ComponentID lastID = 0;
  return lastID++; 
}
constexpr std::size_t maxComponents = 32;
using ComponentBitSet = std::bitset<maxComponents>;
using ComponentArray  = std::array<Component*, maxComponents>;

/**
 * Gets component's type ID.
 * 
 * @exceptsafe Shall not throw execeptions.
 * @param T 
 * @return Component type ID.
 */
template <typename T> inline ComponentID getComponentTypeID() noexcept
{
  static ComponentID typeID = getComponentTypeID();
  return typeID;
}

class Component
{
public:
  Entity* entity;

  virtual ~Component() = default;

  // virtual --means that the function can be override
  virtual void init() {};
  virtual void update(float deltaTime) {}
  virtual void draw() {}
};

class Component;
class Entity
{
private:
  bool active = true;
  std::vector<std::unique_ptr<Component>> componentsVec;

  ComponentArray componentArray;
  ComponentBitSet componentBitSet;

public:
  void update(float deltaTime);
  void draw();

  /**
   * Gets the desired component of the game object giving the type of the component.
   * Example of usage: 
   *         gameobject.getComponent<Transform>().position.x;
   *
   * @tparam T The type of the desired component.
   * @return The pointer to the desired component.
   */
  template <typename T> T &getComponent() const
  {
    auto ptr(componentArray[getComponentTypeID<T>()]);
    return *static_cast<T*>(ptr);
  }

  /**
   * Checks if the game object has the component that you are looking for.
   * Example of usage:
   *          bool existsComponent = gameobject.hasComponent<Transform>();
   *
   * @tparam T The type of component that you are looking for.
   * @return true if the game object has the component that you are looking for.
   *         returns false otherwise.
   */
  template <typename T> bool hasComponent() const
  {
    return componentArray[getComponentTypeID<T>()] != nullptr;
  }

  /**
   * Adds a new component to the desired game object.
   * Example of usage:
   *         gameobject.addComponent<Transform>(new Vector2f(23.0f, 21.0f));
   *
   * @tparam T The type of component that you want to add.
   * @return the new component's pointer.
   */
  template <typename T, typename... TArgs> 
  T &addComponent(TArgs&&... mArgs)
  {
    T* c(new T(std::forward<TArgs>(mArgs)...));
    c->entity = this;
    std::unique_ptr<Component> uPtr { c };
    componentsVec.emplace_back(std::move(uPtr));

    componentArray[getComponentTypeID<T>()] = c;
    componentBitSet[getComponentTypeID<T>()] = true;
            
    c->init();

    return *c;         
  }

  void pop();

  bool isActive();
};

class Manager
{
private:
  std::vector<std::unique_ptr<Entity>> entitiesList;

public:
  void update(float deltaTime);
  void draw();
  void refresh();
  Entity &addEntity();
};