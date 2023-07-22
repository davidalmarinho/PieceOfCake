#include "ECS.hpp"

void Entity::update(float deltaTime)
{
  for (auto &c : componentsVec) c->update(deltaTime);
}

void Entity::draw()
{
  for (auto &c : componentsVec) c->draw();
}

void Entity::pop()
{
  this->active = false;
}

bool Entity::isActive()
{
  return this->active;
}

void Manager::update(float deltaTime)
{
  for (auto &e : entitiesList) e->update(deltaTime);
}

void Manager::draw()
{
  for (auto &e : entitiesList) e->draw();
}

void Manager::refresh()
{
  entitiesList.erase(std::remove_if(std::begin(entitiesList), std::end(entitiesList), [](const std::unique_ptr<Entity> &mEntity)
  {
    return !mEntity->isActive();
  }), std::end(entitiesList)
  );
}

Entity &Manager::addEntity()
{
  Entity *e = new Entity();
  std::unique_ptr<Entity> uPtr{ e };
  entitiesList.emplace_back(std::move(uPtr));
  return *e;
}