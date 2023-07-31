#pragma once

#include <memory>

#include "ECS.hpp"
#include "Texture.hpp"

class TextureRenderer : public Component
{
public:
  std::weak_ptr<Texture> texture; // TODO: Make this private!

  TextureRenderer(std::shared_ptr<Texture> texture);
};