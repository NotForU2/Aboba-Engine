#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "VulkanMesh.hpp"
#include "VulkanContext.hpp"

class AssetManager
{
public:
  void Init(VulkanContext *context);
  VulkanMesh *LoadMesh(const std::string &name, const std::string &filepath);
  VulkanMesh *CreateQuad(const std::string &name, float size);
  VulkanMesh *GetMesh(const std::string &name);
  void Cleanup();

private:
  VulkanContext *mContext = nullptr;
  std::unordered_map<std::string, std::unique_ptr<VulkanMesh>> mMeshes;
};