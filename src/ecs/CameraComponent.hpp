#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CameraComponent
{
  glm::vec3 focusPoint{0.0f, 0.0f, 0.0f};
  float distance = 10.0f;
  float pitch = 45.0f;
  float yaw = 0.0f;
  float fov = 45.0f;

  glm::mat4 GetModelMatrix() const
  {
    float horizontalDist = distance * cos(glm::radians(pitch));
    float verticalDist = distance * sin(glm::radians(pitch));

    float offsetX = horizontalDist * sin(glm::radians(yaw));
    float offsetZ = horizontalDist * cos(glm::radians(yaw));

    glm::vec3 cameraPos = focusPoint - glm::vec3(offsetX, -verticalDist, offsetZ);

    return glm::lookAt(cameraPos, focusPoint, glm::vec3(0.0f, 1.0f, 0.0f));
  }
};

// struct Camera {
//     glm::vec3 focusPoint{0.0f, 0.0f, 0.0f}; // Точка на земле, куда смотрит камера
//     float zoom = 10.0f;     // Высота камеры над землей
//     float angle = 45.0f;    // Угол наклона (обычно 45-60 градусов для RTS)

//     // Матрица вида теперь рассчитывается иначе
//     glm::mat4 GetViewMatrix() const {
//         // Рассчитываем позицию "глаза" камеры относительно точки фокуса
//         // Смещаем камеру назад (по Z) и вверх (по Y)
//         float yOffset = zoom;
//         float zOffset = zoom / tan(glm::radians(angle)); // Или просто zoom, если хотим 45 градусов

//         glm::vec3 eyePosition = focusPoint + glm::vec3(0.0f, yOffset, zOffset);

//         return glm::lookAt(eyePosition, focusPoint, glm::vec3(0.0f, 1.0f, 0.0f));
//     }
// };