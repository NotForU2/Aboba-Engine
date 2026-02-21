#include "Scene.hpp"
#include "../ecs/CameraComponent.hpp"
#include "../ecs/MeshComponent.hpp"
#include "../ecs/TransformComponent.hpp"

void Scene::Init(AssetManager *assetManager)
{
  auto camEntity = mRegistry.create();
  mRegistry.emplace<CameraComponent>(camEntity);

  auto ground = mRegistry.create();
  mRegistry.emplace<TransformComponent>(ground, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(50.0f, 1.0f, 50.0f));
  mRegistry.emplace<MeshComponent>(ground, assetManager->GetMesh("ground"));

  auto unit = mRegistry.create();
  mRegistry.emplace<TransformComponent>(unit, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
  mRegistry.emplace<MeshComponent>(unit, assetManager->GetMesh("panda"));
}

void Scene::Update(float dt) {}

std::vector<RenderObject> Scene::ExtractRenderData()
{
  std::vector<RenderObject> renderQueue;

  auto view = mRegistry.view<TransformComponent, MeshComponent>();
  renderQueue.reserve(view.size_hint());

  for (auto [entity, transform, meshComp] : view.each())
  {
    renderQueue.push_back({meshComp.mesh,
                           transform.GetModelMatrix()});
  }

  return renderQueue;
}

CameraRenderData Scene::ExtractCameraData(float aspectRatio)
{
  CameraRenderData camData{};

  auto camView = mRegistry.view<CameraComponent>();
  if (!camView.empty())
  {
    auto &camera = camView.get<CameraComponent>(camView.front());
    camData.view = camera.GetModelMatrix();
    camData.projection = glm::perspective(glm::radians(camera.fov), aspectRatio, 0.1f, 100.0f);
  }

  return camData;
}