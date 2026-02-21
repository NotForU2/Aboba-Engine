#include "AssetManager.hpp"

void AssetManager::Init(VulkanContext *context)
{
  mContext = context;
}

VulkanMesh *AssetManager::LoadMesh(const std::string &name, const std::string &filepath)
{
  if (mMeshes.find(name) != mMeshes.end())
  {
    return mMeshes[name].get();
  }

  auto mesh = std::make_unique<VulkanMesh>();
  mesh->LoadFromFile(mContext, filepath);
  mMeshes[name] = std::move(mesh);

  return mMeshes[name].get();
}

VulkanMesh *AssetManager::CreateQuad(const std::string &name, float size)
{
  if (mMeshes.find(name) != mMeshes.end())
  {
    return mMeshes[name].get();
  }

  auto mesh = std::make_unique<VulkanMesh>();
  mesh->CreateQuad(mContext, size);
  mMeshes[name] = std::move(mesh);

  return mMeshes[name].get();
}

VulkanMesh *AssetManager::GetMesh(const std::string &name)
{
  auto it = mMeshes.find(name);
  return it != mMeshes.end() ? it->second.get() : nullptr;
}

void AssetManager::Cleanup()
{
  for (auto &pair : mMeshes)
  {
    pair.second->Destroy(mContext);
  }
  mMeshes.clear();
}