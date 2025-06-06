// CCup.cpp
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include "CCup.h"
#include "../common/typedefs.h"

CCup::CCup(float bottomR, float topR, float height,
    unsigned int radialSegs, unsigned int heightSegs, GLuint pattern)
    : CShape() {
    generateCup(bottomR, topR, height, radialSegs, heightSegs, pattern);
}

CCup::~CCup() {
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
    glDeleteVertexArrays(1, &_vao);
    if (_points) delete[] _points;
    if (_idx)    delete[] _idx;
}

void CCup::draw() {
    glUseProgram(_shaderProg);
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CCup::drawRaw() {
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CCup::update(float dt) {
    // 可加動畫
}

void CCup::generateCup(float bottomR, float topR, float height,
    unsigned int radialSegs, unsigned int heightSegs, GLuint pattern) {
    _vtxAttrCount = 11;
    unsigned int sideVerts = (heightSegs + 1) * (radialSegs + 1);
    unsigned int bottomVerts = 1 + (radialSegs + 1);
    _vtxCount = sideVerts + bottomVerts;
    _idxCount = heightSegs * radialSegs * 6 + radialSegs * 3;

    _points = new GLfloat[_vtxCount * _vtxAttrCount];
    _idx = new GLuint[_idxCount];

    glm::vec3 baseColor, topColor;
    switch (pattern) {
    case 2: baseColor = { 1.0f, 0.85f, 0.7f }; topColor = { 0.9f, 0.7f, 0.5f }; break;
    case 3: baseColor = { 0.9f, 0.9f, 0.6f }; topColor = { 0.7f, 0.8f, 0.3f }; break;
    case 4: baseColor = { 0.85f, 0.75f, 0.85f }; topColor = { 0.65f, 0.5f, 0.85f }; break;
    default: baseColor = { 0.8f, 0.9f, 1.0f }; topColor = { 0.6f, 0.8f, 0.95f }; break;
    }

    unsigned int vi = 0;
    for (unsigned int i = 0; i <= heightSegs; ++i) {
        float v = float(i) / heightSegs;
        float r = bottomR * (1 - v) + topR * v;
        float y = v * height;
        glm::vec3 color = glm::mix(baseColor, topColor, v);

        for (unsigned int j = 0; j <= radialSegs; ++j) {
            float u = float(j) / radialSegs;
            float theta = u * 2.0f * M_PI;
            float x = r * cosf(theta);
            float z = -r * sinf(theta);
            glm::vec3 pos(x, y, z);
            glm::vec3 normal = glm::normalize(glm::vec3(cosf(theta), 0.0f, -sinf(theta)));
            glm::vec2 uv(u, v);

            _points[vi * _vtxAttrCount + 0] = pos.x;
            _points[vi * _vtxAttrCount + 1] = pos.y;
            _points[vi * _vtxAttrCount + 2] = pos.z;
            _points[vi * _vtxAttrCount + 3] = color.r;
            _points[vi * _vtxAttrCount + 4] = color.g;
            _points[vi * _vtxAttrCount + 5] = color.b;
            _points[vi * _vtxAttrCount + 6] = normal.x;
            _points[vi * _vtxAttrCount + 7] = normal.y;
            _points[vi * _vtxAttrCount + 8] = normal.z;
            _points[vi * _vtxAttrCount + 9] = uv.x;
            _points[vi * _vtxAttrCount + 10] = uv.y;
            ++vi;
        }
    }

    unsigned int centerIdx = vi;
    _points[vi * _vtxAttrCount + 0] = 0;
    _points[vi * _vtxAttrCount + 1] = 0;
    _points[vi * _vtxAttrCount + 2] = 0;
    _points[vi * _vtxAttrCount + 3] = baseColor.r;
    _points[vi * _vtxAttrCount + 4] = baseColor.g;
    _points[vi * _vtxAttrCount + 5] = baseColor.b;
    _points[vi * _vtxAttrCount + 6] = 0;
    _points[vi * _vtxAttrCount + 7] = -1;
    _points[vi * _vtxAttrCount + 8] = 0;
    _points[vi * _vtxAttrCount + 9] = 0.5f;
    _points[vi * _vtxAttrCount + 10] = 0.5f;
    ++vi;

    for (unsigned int j = 0; j <= radialSegs; ++j) {
        float u = float(j) / radialSegs;
        float theta = u * 2.0f * M_PI;
        float x = bottomR * cosf(theta);
        float z = -bottomR * sinf(theta);
        _points[vi * _vtxAttrCount + 0] = x;
        _points[vi * _vtxAttrCount + 1] = 0;
        _points[vi * _vtxAttrCount + 2] = z;
        _points[vi * _vtxAttrCount + 3] = baseColor.r;
        _points[vi * _vtxAttrCount + 4] = baseColor.g;
        _points[vi * _vtxAttrCount + 5] = baseColor.b;
        _points[vi * _vtxAttrCount + 6] = 0;
        _points[vi * _vtxAttrCount + 7] = -1;
        _points[vi * _vtxAttrCount + 8] = 0;
        _points[vi * _vtxAttrCount + 9] = 0.5f + 0.5f * cosf(theta);
        _points[vi * _vtxAttrCount + 10] = 0.5f + 0.5f * sinf(theta);
        ++vi;
    }

    unsigned int ii = 0;
    for (unsigned int i = 0; i < heightSegs; ++i) {
        for (unsigned int j = 0; j < radialSegs; ++j) {
            unsigned int a = i * (radialSegs + 1) + j;
            unsigned int b = a + (radialSegs + 1);
            _idx[ii++] = a; _idx[ii++] = b; _idx[ii++] = a + 1;
            _idx[ii++] = a + 1; _idx[ii++] = b; _idx[ii++] = b + 1;
        }
    }
    for (unsigned int j = 0; j < radialSegs; ++j) {
        _idx[ii++] = centerIdx;
        _idx[ii++] = centerIdx + 1 + j;
        _idx[ii++] = centerIdx + 1 + j + 1;
    }
}
