#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Arcball {
private:
    bool _isDragging;
    glm::vec2 _lastMousePos;
    glm::quat _currentRotation;
    glm::vec3 mapToArcball(glm::vec2 screenPos, int width, int height);
    float _speed;

public:
    Arcball();
    void onMouseButton(int button, int action, double xpos, double ypos);
    void onCursorMove(double xpos, double ypos, int width, int height);
    glm::quat getRotation() const { return _currentRotation;  }
    void setSpeed(float sp) { _speed = sp; }
    void reset() { _currentRotation  = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);  }
};
