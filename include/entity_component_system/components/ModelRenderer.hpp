#pragma once

#include "ECS.hpp"

#include "Model.hpp"

class ModelRenderer : public Component
{
public:
  std::weak_ptr<Model> model; // TODO: Make this private!

  ModelRenderer(std::shared_ptr<Model> model);
};