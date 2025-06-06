#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <memory>

#include "CCamera.h"
#include "wmhandler.h"
#include "arcball.h"

#include "../common/CMaterial.h"
#include "../models/CQuad.h"
#include "../models/CCube.h"
#include "../models/CSphere.h"
#include "../models/CTeapot.h"
#include "../common/CLight.h"
#include "../common/CButton.h"
#include "../common/Model.h"
#include "CollisionManager.h"

//#define SPOT_TARGET  // Example 2

extern Arcball g_arcball;

bool  g_bCamRoting = false;
bool  g_bfirstMouse = true;
float g_lastX = 400, g_lastY = 400;
float g_mouseSens = 0.005f;

extern CCube g_centerloc;
extern GLuint g_shadingProg;
extern glm::vec3 g_eyeloc;
extern CLight* g_light;
extern CLight* pointLight1;
extern CLight* spotLight1;
extern CLight* spotLight2;
extern CLight* spotLight3;

extern std::array<CButton, 4> g_button;
extern std::vector<std::unique_ptr<Model>> models;

extern CMaterial g_matWaterGreen;
extern CSphere  g_sphere;

#ifdef SPOT_TARGET
extern CCube g_spotTarget;
#endif

extern CollisionManager g_collisionManager;
Arcball g_arcball;

// 新增：計算攝影機的前方、右方、上方向量
glm::vec3 getCameraForward() {
    // 從攝影機的view matrix計算前方向量
    glm::mat4 viewMatrix = CCamera::getInstance().getViewMatrix();
    // view matrix的第三列是前方向量（需要反向）
    return -glm::vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]);
}

glm::vec3 getCameraRight() {
    // 從攝影機的view matrix計算右方向量
    glm::mat4 viewMatrix = CCamera::getInstance().getViewMatrix();
    // view matrix的第一列是右方向量
    return glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
}

glm::vec3 getCameraUp() {
    // 從攝影機的view matrix計算上方向量
    glm::mat4 viewMatrix = CCamera::getInstance().getViewMatrix();
    // view matrix的第二列是上方向量
    return glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);
}


void updateCameraPosition(const glm::vec3& movement) {
    glm::vec3 currentEye = CCamera::getInstance().getViewLocation();
    

    glm::vec3 safeMovement = g_collisionManager.getSafeMovement(movement, currentEye);
    glm::vec3 newEye = currentEye + safeMovement;

    glm::vec3 forward = getCameraForward();
    float distance = glm::length(g_centerloc.getPos() - currentEye);
    glm::vec3 newCenter = newEye + forward * distance;
    
    g_eyeloc = newEye;
    g_centerloc.setPos(newCenter);
    std::cout << "After move: Eye(" << g_eyeloc.x << ", " << g_eyeloc.y << ", " << g_eyeloc.z << ") Center(" << g_centerloc.getPos().x << ", " << g_centerloc.getPos().y << ", " << g_centerloc.getPos().z << ")" << std::endl;

    CCamera::getInstance().updateViewCenter(g_eyeloc, g_centerloc.getPos());
    
    glm::mat4 mxView = CCamera::getInstance().getViewMatrix();
    GLint viewLoc = glGetUniformLocation(g_shadingProg, "mxView");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mxView));
}

void initializeCollisionSystem() {
    g_collisionManager.addObstacle(AABB(glm::vec3(-1.0f, -2.0f, -1.0f),
                                       glm::vec3(1.0f, 0.0f, 1.0f)));
    
    g_collisionManager.addObstacle(AABB(glm::vec3(2.0f, -2.0f, -2.0f),
                                       glm::vec3(3.0f, 0.0f, -1.0f)));
    
    std::cout << "Collision system initialized with "
              << g_collisionManager.getWallCount() << " walls and "
              << g_collisionManager.getObstacleCount() << " obstacles." << std::endl;
}

bool checkModelCollision(const glm::vec3& modelPosition, const glm::vec3& modelSize) {
    AABB modelAABB(modelPosition - modelSize * 0.5f,
                   modelPosition + modelSize * 0.5f);
    
    // 這裡可以實現模型與模型之間的碰撞檢測
    // 暫時返回false，可以根據需要擴展
    return false;
}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    // Arcball* arcball = static_cast<Arcball*>(glfwGetWindowUserPointer(window));
    g_arcball.onMouseButton(button, action, xpos, ypos);
    //std::cout << "button = " << button << "action = " << action << "mods = " << mods << std::endl;
    if (button == GLFW_MOUSE_BUTTON_LEFT )
    {
        if (action == GLFW_PRESS)
        {
            g_bCamRoting = true;
            if (g_button[0].handleClick((float)xpos, height - (float)ypos)) {
                if(g_light->isLightOn() == true)
                {
                    g_light->setLightOn(false);
                }else {
                    g_light->setLightOn(true);
                }
            }
            if (g_button[1].handleClick((float)xpos, height - (float)ypos)) {
                if(spotLight1->isLightOn() == true){
                    spotLight1->setLightOn(false);
                }else{
                    spotLight1->setLightOn(true);
                }
            }
            if (g_button[2].handleClick((float)xpos, height - (float)ypos)) {
                if(spotLight2->isLightOn() == true){
                    spotLight2->setLightOn(false);
                }else{
                    spotLight2->setLightOn(true);
                }
            }
            if (g_button[3].handleClick((float)xpos, height - (float)ypos)) {
                if(spotLight3->isLightOn() == true){
                    spotLight3->setLightOn(false);
                }else{
                    spotLight3->setLightOn(true);
                }
            }
        }
        else if (action == GLFW_RELEASE)
        {
            g_bCamRoting = false;
            g_bfirstMouse = true;
        }
    }
}

// ---------------------------------------------------------------------------------------


void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    g_arcball.onCursorMove(xpos, ypos, width, height);
    
    if (g_bCamRoting) {
        if (g_bfirstMouse) {
            g_lastX = (float)xpos;
            g_lastY = (float)ypos;
            g_bfirstMouse = false;
            return;
        }
        
        float deltaX = ((float)xpos - g_lastX);
        float deltaY = ((float)ypos - g_lastY);
        g_lastX = (float)xpos;
        g_lastY = (float)ypos;

        CCamera::getInstance().processMouseMovement(deltaX, deltaY, g_mouseSens);
        g_eyeloc = CCamera::getInstance().getViewLocation();
        
        const float LOOK_DISTANCE = 3.0f;
        
        glm::vec3 forward = getCameraForward();
        glm::vec3 newCenter = g_eyeloc + forward * LOOK_DISTANCE;
        
        glm::vec3 safeCenterMovement = g_collisionManager.getSafeMovement(
            newCenter - g_centerloc.getPos(), g_centerloc.getPos());
        
        g_centerloc.setPos(g_centerloc.getPos() + safeCenterMovement);
        
        CCamera::getInstance().updateViewCenter(g_eyeloc, g_centerloc.getPos());
        
        std::cout << "After rotation: Eye(" << g_eyeloc.x << ", " << g_eyeloc.y
                  << ", " << g_eyeloc.z << ") Center(" << g_centerloc.getPos().x
                  << ", " << g_centerloc.getPos().y << ", " << g_centerloc.getPos().z
                  << ")" << std::endl;
        
        glm::mat4 mxView = CCamera::getInstance().getViewMatrix();
        GLint viewLoc = glGetUniformLocation(g_shadingProg, "mxView");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mxView));
    }
}
// ---------------------------------------------------------------------------------------

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// ---------------------------------------------------------------------------------------
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {

    CCamera::getInstance().updateRadius((float)yoffset * -0.2f);
    glm::mat4 mxView = CCamera::getInstance().getViewMatrix();
    GLint viewLoc = glGetUniformLocation(g_shadingProg, "mxView");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mxView));
    g_eyeloc = CCamera::getInstance().getViewLocation();

    //std::cout << "Scroll event: xoffset = " << xoffset << ", yoffset = " << yoffset << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    glm::vec3 vPos;
    glm::mat4 mxView, mxProj;
    GLint viewLoc, projLoc;
    float shin;

    // 移動速度常數
    const float moveSpeed = 0.1f;

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS) { glfwSetWindowShouldClose(window, true); }
            break;
        case GLFW_KEY_SPACE:
            break;
#ifdef SPOT_TARGET
        case 262:
            vPos = g_spotTarget.getPos();
            g_spotTarget.setPos(glm::vec3(vPos.x + 0.15f, vPos.y, vPos.z));
            g_light.setTarget(g_spotTarget.getPos());
            break;
        case 263:
            vPos = g_spotTarget.getPos();
            g_spotTarget.setPos(glm::vec3(vPos.x - 0.15f, vPos.y, vPos.z));
            g_light.setTarget(g_spotTarget.getPos());
            break;
        case 264:
            vPos = g_spotTarget.getPos();
            g_spotTarget.setPos(glm::vec3(vPos.x, vPos.y, vPos.z - 0.15f));
            g_light.setTarget(g_spotTarget.getPos());
            break;
        case 265:
            vPos = g_spotTarget.getPos();
            g_spotTarget.setPos(glm::vec3(vPos.x, vPos.y, vPos.z + 0.15f));
            g_light.setTarget(g_spotTarget.getPos());
            break;
#endif
        default:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                bool isShiftPressed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
                                      (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
                if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
                    char letter = (isShiftPressed) ? ('A' + (key - GLFW_KEY_A)) : ('a' + (key - GLFW_KEY_A));
                    std::cout << "key = " << letter << std::endl;
                    switch (letter) {
                        case 'r': // diffuse（紅色調）
                            spotLight1->setDiffuse(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
                            spotLight2->setDiffuse(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
                            spotLight3->setDiffuse(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
                            pointLight1->setDiffuse(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
                            break;
                        case 'R':
                            spotLight1->setDiffuse(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
                            spotLight2->setDiffuse(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
                            spotLight3->setDiffuse(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
                            pointLight1->setDiffuse(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
                            break;
                        case 'b': // diffuse（藍色調）
                            spotLight1->setDiffuse(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f));
                            spotLight2->setDiffuse(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f));
                            spotLight3->setDiffuse(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f));
                            pointLight1->setDiffuse(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f));
                            break;
                        case 'B':
                            spotLight1->setDiffuse(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f));
                            spotLight2->setDiffuse(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f));
                            spotLight3->setDiffuse(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f));
                            pointLight1->setDiffuse(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f));
                            break;
                        case 'g': // diffuse（綠色調）
                            spotLight1->setDiffuse(glm::vec4(0.3f, 0.8f, 0.5f, 1.0f));
                            spotLight2->setDiffuse(glm::vec4(0.3f, 0.8f, 0.5f, 1.0f));
                            spotLight3->setDiffuse(glm::vec4(0.3f, 0.8f, 0.5f, 1.0f));
                            pointLight1->setDiffuse(glm::vec4(0.3f, 0.8f, 0.5f, 1.0f));
                            break;
                        case 'G':
                            spotLight1->setDiffuse(glm::vec4(0.3f, 0.8f, 0.5f, 1.0f));
                            spotLight2->setDiffuse(glm::vec4(0.3f, 0.8f, 0.5f, 1.0f));
                            spotLight3->setDiffuse(glm::vec4(0.3f, 0.8f, 0.5f, 1.0f));
                            pointLight1->setDiffuse(glm::vec4(0.3f, 0.8f, 0.5f, 1.0f));
                            break;
                        case 'n': // diffuse（白色調）
                            spotLight1->setDiffuse(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
                            spotLight2->setDiffuse(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
                            spotLight3->setDiffuse(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
                            pointLight1->setDiffuse(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
                            break;
                        case 'N':
                            spotLight1->setDiffuse(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
                            spotLight2->setDiffuse(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
                            spotLight3->setDiffuse(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
                            pointLight1->setDiffuse(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
                            break;
                        case 'P':
                        case 'p':
                            if (CCamera::getInstance().getProjectionType() != CCamera::Type::PERSPECTIVE) {
                                CCamera::getInstance().updatePerspective(45.0f, 1.0f, 1.0f, 100.0f);
                                mxProj = CCamera::getInstance().getProjectionMatrix();
                                projLoc = glGetUniformLocation(g_shadingProg, "mxProj");
                                glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(mxProj));
                            }
                            break;
                        case 'O':
                        case 'o':
                            if (CCamera::getInstance().getProjectionType() != CCamera::Type::ORTHOGRAPHIC) {
                                CCamera::getInstance().updateOrthographic(-3.0f, 3.0f, -3.0f, 3.0f, 1.0f, 100.0f);
                                mxProj = CCamera::getInstance().getProjectionMatrix();
                                projLoc = glGetUniformLocation(g_shadingProg, "mxProj");
                                glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(mxProj));
                            }
                            break;
                        case 'W':
                        case 'w':
//                            vPos = g_centerloc.getPos();
//                            g_centerloc.setPos(glm::vec3(vPos.x, vPos.y , vPos.z - 0.05f));
//                            g_eyeloc = CCamera::getInstance().getViewLocation();
//                            g_eyeloc.z = g_eyeloc.z - 0.05f;
//                            CCamera::getInstance().updateViewCenter(g_eyeloc, g_centerloc.getPos());
//                            mxView = CCamera::getInstance().getViewMatrix();
//                            viewLoc = glGetUniformLocation(g_shadingProg, "mxView");
//                            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mxView));
                            // 向前移動（沿著攝影機前方向量）
                            {
                                glm::vec3 forward = getCameraForward();
                                glm::vec3 movement = forward * moveSpeed;
                                updateCameraPosition(movement);
                            }
                            break;
                        case 'S':
                        case 's':
//                            vPos = g_centerloc.getPos();
//                            g_centerloc.setPos(glm::vec3(vPos.x , vPos.y , vPos.z + 0.05f));
//                            g_eyeloc = CCamera::getInstance().getViewLocation();
//                            g_eyeloc.z = g_eyeloc.z + 0.05f;
//                            CCamera::getInstance().updateViewCenter(g_eyeloc, g_centerloc.getPos());
//                            mxView = CCamera::getInstance().getViewMatrix();
//                            viewLoc = glGetUniformLocation(g_shadingProg, "mxView");
//                            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mxView));
                            // 向後移動（沿著攝影機前方向量的反方向）
                            {
                                glm::vec3 forward = getCameraForward();
                                glm::vec3 movement = -forward * moveSpeed;
                                updateCameraPosition(movement);
                            }
                            break;
                        case 'A':
                        case 'a':
//                            vPos = g_centerloc.getPos();
//                            g_centerloc.setPos(glm::vec3(vPos.x - 0.05f, vPos.y, vPos.z));
//                            g_eyeloc = CCamera::getInstance().getViewLocation();
//                            g_eyeloc.x = g_eyeloc.x - 0.05f;
//                            CCamera::getInstance().updateViewCenter(g_eyeloc, g_centerloc.getPos());
//                            mxView = CCamera::getInstance().getViewMatrix();
//                            viewLoc = glGetUniformLocation(g_shadingProg, "mxView");
//                            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mxView));
                            // 向左移動（沿著攝影機右方向量的反方向）
                            {
                                glm::vec3 right = getCameraRight();
                                glm::vec3 movement = -right * moveSpeed;
                                updateCameraPosition(movement);
                            }
                            break;
                        case 'D':
                        case 'd':
//                            vPos = g_centerloc.getPos();
//                            g_centerloc.setPos(glm::vec3(vPos.x + 0.05f, vPos.y, vPos.z));
//                            g_eyeloc = CCamera::getInstance().getViewLocation();
//                            g_eyeloc.x = g_eyeloc.x + 0.05f;
//                            CCamera::getInstance().updateViewCenter(g_eyeloc, g_centerloc.getPos());
//                            mxView = CCamera::getInstance().getViewMatrix();
//                            viewLoc = glGetUniformLocation(g_shadingProg, "mxView");
//                            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mxView));
                            // 向右移動（沿著攝影機右方向量）
                            {
                                glm::vec3 right = getCameraRight();
                                glm::vec3 movement = right * moveSpeed;
                                updateCameraPosition(movement);
                            }
                            break;
                        case 'Q':
                        case 'q':
                            // 向上移動
                            {
                                glm::vec3 up = getCameraUp();
                                glm::vec3 movement = up * moveSpeed;
                                updateCameraPosition(movement);
                            }
                            break;
                        case 'E':
                        case 'e':
                            // 向下移動
                            {
                                glm::vec3 up = getCameraUp();
                                glm::vec3 movement = -up * moveSpeed;
                                updateCameraPosition(movement);
                            }
                            break;
                        case 'L':
                        case 'l':
                            g_light->setMotionEnabled();
//                            models[8]->setAutoRotate();
                            break;
                        case 'C':
                        case 'c':
                            // 新增：調整攝影機碰撞半徑
                            {
                                static bool largeBoundary = false;
                                if (largeBoundary) {
                                    g_collisionManager.setCameraRadius(0.3f);
                                    std::cout << "Camera collision radius: 0.3" << std::endl;
                                } else {
                                    g_collisionManager.setCameraRadius(0.6f);
                                    std::cout << "Camera collision radius: 0.6" << std::endl;
                                }
                                largeBoundary = !largeBoundary;
                            }
                            break;
                    }
                }
            }
        
    }
}
