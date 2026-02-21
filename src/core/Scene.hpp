#pragma once
#include <entt/entt.hpp>
#include "../graphics/AssetManager.hpp"
#include "../graphics/VulkanRenderer.hpp"

class Scene
{
public:
  void Init(AssetManager *assetManager);
  void Update(float dt);
  std::vector<RenderObject> ExtractRenderData();
  CameraRenderData ExtractCameraData(float aspectRatio);
  entt::registry &GetRegistry() { return mRegistry; }

private:
  entt::registry mRegistry;
};