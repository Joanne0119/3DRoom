#include "arcball.h"

Arcball::Arcball()
{
    _isDragging = false;
    _currentRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    _lastMousePos = glm::vec2(0, 0);
    _speed = 1.0f;
}

glm::vec3 Arcball::mapToArcball(glm::vec2 screenPos, int width, int height) {
    float x = (2.0f * screenPos.x - width) / width;
    float y = (height - 2.0f * screenPos.y) / height;
    float z = 0.0f;
    float lengthSquared = x * x + y * y;
    if (lengthSquared <= 1.0f) {
        z = sqrt(1.0f - lengthSquared);
    }
    else {
        float length = sqrt(lengthSquared);
        x /= length;
        y /= length;
    }
    return glm::normalize(glm::vec3(x, y, z));
}


void Arcball::onMouseButton(int button, int action, double xpos, double ypos) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            _isDragging = true;
            _lastMousePos = glm::vec2(xpos, ypos);
        }
        else if (action == GLFW_RELEASE) {
            _isDragging = false;
        }
    }
}

void Arcball::onCursorMove(double xpos, double ypos, int width, int height) {
    if (_isDragging) {
        glm::vec2 currentMousePos = glm::vec2(xpos, ypos);
        glm::vec3 va = mapToArcball(_lastMousePos, width, height);
        glm::vec3 vb = mapToArcball(currentMousePos, width, height);

        float dot = glm::dot(va, vb);
        float angle = acos(glm::clamp(dot, -1.0f, 1.0f)) * _speed;
        glm::vec3 axis = glm::cross(va, vb);
        if (glm::length(axis) > 0.0001f) {
            axis = glm::normalize(axis);
            float halfAngle = angle * 0.5f;
            float s = sin(halfAngle);
            glm::quat rotation(cos(halfAngle), s * axis.x, s * axis.y, s * axis.z);
            _currentRotation = rotation * _currentRotation;
        }
        _lastMousePos = currentMousePos;
    }

}