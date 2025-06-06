#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#include "CDonut.h"
#include "../common/typedefs.h" 

CDonut::CDonut(float majorRadius, float minorRadius, GLuint segments, GLuint slices, GLuint pattern)
    : CShape() {
    generateDonut(majorRadius, minorRadius, segments, slices, pattern);
}

CDonut::~CDonut() {
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
    glDeleteVertexArrays(1, &_vao);
    if (_points) delete[] _points;
    if (_idx)    delete[] _idx;
}

void CDonut::draw() {
    glUseProgram(_shaderProg);
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CDonut::drawRaw() {
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CDonut::reset() {
    CShape::reset();
}

void CDonut::update(float dt) {
    // 可加入動畫或物理行為
}

void CDonut::generateDonut(float majorRadius, float minorRadius, GLuint segments, GLuint slices, GLuint pattern) {
    _vtxAttrCount = 11;
    _vtxCount = (segments + 1) * (slices + 1);
    _idxCount = segments * slices * 6;

    _points = new GLfloat[_vtxCount * _vtxAttrCount];
    _idx = new GLuint[_idxCount];

    // 四種顏色方案
    glm::vec4 color1, color2;
    switch (pattern) {
    case 1:
        // 夕陽漸層
        color1 = glm::vec4(1.00f, 0.85f, 0.75f, 1.0f);
        color2 = glm::vec4(1.00f, 0.65f, 0.45f, 1.0f);
        break;
    case 2:
        // 薰衣草漸層
        color1 = glm::vec4(0.85f, 0.75f, 0.95f, 1.0f);
        color2 = glm::vec4(0.65f, 0.55f, 0.75f, 1.0f);
        break;
    case 3:
        // 薄荷漸層
        color1 = glm::vec4(0.75f, 0.95f, 0.85f, 1.0f);
        color2 = glm::vec4(0.55f, 0.75f, 0.65f, 1.0f);
        break;
    case 4:
        // 玫瑰漸層
        color1 = glm::vec4(0.95f, 0.75f, 0.85f, 1.0f);
        color2 = glm::vec4(0.75f, 0.55f, 0.65f, 1.0f);
        break;
    default:
        color1 = glm::vec4(1.00f, 0.85f, 0.75f, 1.0f);
        color2 = glm::vec4(1.00f, 0.65f, 0.45f, 1.0f);
    }

    float segStep = 2.0f * M_PI / segments;
    float sliceStep = 2.0f * M_PI / slices;

    GLuint vi = 0;
    for (GLuint i = 0; i <= segments; ++i) {
        float u = float(i) / segments;
        float theta = i * segStep;
        glm::vec3 center(cos(theta) * majorRadius, 0.0f, -sin(theta) * majorRadius);

        for (GLuint j = 0; j <= slices; ++j) {
            float v = float(j) / slices;
            float phi = j * sliceStep;
            glm::vec3 pos(
                (majorRadius + minorRadius * cos(phi)) * cos(theta),
                minorRadius * sin(phi),
                -(majorRadius + minorRadius * cos(phi)) * sin(theta)
            );
            glm::vec3 normal = glm::normalize(glm::vec3(pos.x - center.x, pos.y - center.y, pos.z - center.z));
            glm::vec2 tex = glm::vec2(u, v);

            // 插值顏色 + 微噪聲
            glm::vec4 base = glm::mix(color1, color2, v);
            float noise = 0.05f * sinf(8.0f * u * M_PI) * cosf(6.0f * v * M_PI);
            glm::vec4 color = glm::clamp(base + glm::vec4(noise), 0.0f, 1.0f);

            _points[vi * _vtxAttrCount + 0] = pos.x;
            _points[vi * _vtxAttrCount + 1] = pos.y;
            _points[vi * _vtxAttrCount + 2] = pos.z;
            _points[vi * _vtxAttrCount + 3] = color.r;
            _points[vi * _vtxAttrCount + 4] = color.g;
            _points[vi * _vtxAttrCount + 5] = color.b;
            _points[vi * _vtxAttrCount + 6] = normal.x;
            _points[vi * _vtxAttrCount + 7] = normal.y;
            _points[vi * _vtxAttrCount + 8] = normal.z;
            _points[vi * _vtxAttrCount + 9] = tex.x;
            _points[vi * _vtxAttrCount + 10] = tex.y;
            ++vi;
        }
    }

    GLuint ii = 0;
    for (GLuint i = 0; i < segments; ++i) {
        for (GLuint j = 0; j < slices; ++j) {
            GLuint first = i * (slices + 1) + j;
            GLuint second = first + slices + 1;
            _idx[ii++] = first;
            _idx[ii++] = second;
            _idx[ii++] = first + 1;
            _idx[ii++] = first + 1;
            _idx[ii++] = second;
            _idx[ii++] = second + 1;
        }
    }
}
