#pragma once
#include <GLFW/glfw3.h>

// 滑鼠按鈕按下後 callback function(回呼函式)

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

void framebufferSizeCallback(GLFWwindow* window, int width, int height);

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

glm::vec3 getCameraForward();
glm::vec3 getCameraRight();
glm::vec3 getCameraUp();
void updateCameraPosition(const glm::vec3& movement);
void initializeCollisionSystem();
bool checkModelCollision(const glm::vec3& modelPosition, const glm::vec3& modelSize);
