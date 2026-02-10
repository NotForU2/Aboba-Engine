#pragma once
#define GML_FORCE_RADIUS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
  Camera()
  {
    UpdateViewMatrix();
  }
  void SetPosition(const glm::vec3 &position)
  {
    mPosition = position;
    UpdateViewMatrix();
  }
  void SetTarget(const glm::vec3 &target)
  {
    mTarget = target;
    UpdateViewMatrix();
  }
  void SetProjection(float fov, float aspect, float nearPlane, float farPlane)
  {
    mProjection = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    // В Vulkan Y растет вниз. Нужно перевернуть ось Y в матрице проекции.
    mProjection[1][1] *= -1;
  }
  const glm::mat4 &GetView() const { return mView; }
  const glm::mat4 &GetProjection() const { return mProjection; }
  void Move(const glm::vec3 &delta)
  {
    mPosition += delta;
    mTarget += delta;
    UpdateViewMatrix();
  }

private:
  glm::vec3 mPosition = glm::vec3(2.0f, 2.0f, 2.0f);
  glm::vec3 mTarget = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 mUp = glm::vec3(0.0f, 0.0f, 1.0f);

  glm::mat4 mView{1.0f};
  glm::mat4 mProjection{1.0f};

  void UpdateViewMatrix()
  {
    mView = glm::lookAt(mPosition, mTarget, mUp);
  }
};