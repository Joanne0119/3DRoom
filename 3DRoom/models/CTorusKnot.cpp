// CTorusKnot.cpp (Y 軸朝上)
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include "CTorusKnot.h"
#include "../common/typedefs.h"

CTorusKnot::CTorusKnot(unsigned int p, unsigned int q, float radius, float tube, unsigned int segments, unsigned int sides, GLuint pattern)
    : CShape() {
    generateTorusKnot(p, q, radius, tube, segments, sides, pattern);
}

CTorusKnot::~CTorusKnot() {
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
    glDeleteVertexArrays(1, &_vao);
    delete[] _points;
    delete[] _idx;
}

void CTorusKnot::draw() {
    glUseProgram(_shaderProg);
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CTorusKnot::drawRaw() {
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CTorusKnot::update(float dt) {}


void CTorusKnot::generateTorusKnot(unsigned int p, unsigned int q, float R, float r, unsigned int segments, unsigned int sides, GLuint pattern) {
    _vtxAttrCount = 11;
    _vtxCount = (segments + 1) * (sides + 1);
    _idxCount = segments * sides * 6;
    _points = new GLfloat[_vtxCount * _vtxAttrCount];
    _idx = new GLuint[_idxCount];

    glm::vec3 baseColor, topColor;
    switch (pattern) {
    case 2: baseColor = { 1.0f, 0.85f, 0.7f }; topColor = { 0.9f, 0.7f, 0.5f }; break;
    case 3: baseColor = { 0.9f, 0.9f, 0.6f }; topColor = { 0.7f, 0.8f, 0.3f }; break;
    case 4: baseColor = { 0.85f, 0.75f, 0.85f }; topColor = { 0.65f, 0.5f, 0.85f }; break;
    default: baseColor = { 0.8f, 0.9f, 1.0f }; topColor = { 0.6f, 0.8f, 0.95f }; break;
    }

    auto getPos = [&](float t) {
        float phi = t * 2.0f * M_PI;
        float x = (R + cosf(q * phi)) * cosf(p * phi);
        float y = (R + cosf(q * phi)) * sinf(p * phi);
        float z = sinf(q * phi);
        return glm::vec3(x, z * R, -y); // Y-up
        };

    auto getTangent = [&](float t) {
        float phi = t * 2.0f * M_PI;
        float dx = -(p * (R + cosf(q * phi)) * sinf(p * phi)) - q * sinf(q * phi) * cosf(p * phi);
        float dy = p * (R + cosf(q * phi)) * cosf(p * phi) - q * sinf(q * phi) * sinf(p * phi);
        float dz = q * cosf(q * phi);
        return glm::normalize(glm::vec3(dx, dz * R, -dy)); // Y-up
        };

    GLuint vi = 0;
    for (unsigned int i = 0; i <= segments; ++i) {
        float t = (float)i / segments;
        glm::vec3 center = getPos(t);
        glm::vec3 T = getTangent(t);
        glm::vec3 N = glm::normalize(glm::cross(glm::vec3(0, 1, 0), T));
        glm::vec3 B = glm::normalize(glm::cross(T, N));
        // moved into inner loop

        for (unsigned int j = 0; j <= sides; ++j) {
            float s = (float)j / sides;
            float angle = s * 2.0f * M_PI;
            float globalT = (float(i) + float(j) / sides) / segments;
            glm::vec3 color = glm::mix(baseColor, topColor, globalT);
            float dx = cosf(angle) * r;
            float dy = sinf(angle) * r;
            glm::vec3 pos = center + N * dx + B * dy;
            glm::vec3 normal = glm::normalize(pos - center);

            _points[vi * _vtxAttrCount + 0] = pos.x;
            _points[vi * _vtxAttrCount + 1] = pos.y;
            _points[vi * _vtxAttrCount + 2] = pos.z;
            _points[vi * _vtxAttrCount + 3] = color.r;
            _points[vi * _vtxAttrCount + 4] = color.g;
            _points[vi * _vtxAttrCount + 5] = color.b;
            _points[vi * _vtxAttrCount + 6] = normal.x;
            _points[vi * _vtxAttrCount + 7] = normal.y;
            _points[vi * _vtxAttrCount + 8] = normal.z;
            _points[vi * _vtxAttrCount + 9] = s;
            _points[vi * _vtxAttrCount + 10] = t;
            ++vi;
        }
    }

    GLuint ii = 0;
    for (unsigned int i = 0; i < segments; ++i) {
        for (unsigned int j = 0; j < sides; ++j) {
            GLuint cur = i * (sides + 1) + j;
            GLuint next = cur + sides + 1;
            _idx[ii++] = cur;
            _idx[ii++] = next;
            _idx[ii++] = cur + 1;
            _idx[ii++] = cur + 1;
            _idx[ii++] = next;
            _idx[ii++] = next + 1;
        }
    }
}
