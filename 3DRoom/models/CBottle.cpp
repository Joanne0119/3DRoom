#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include "CBottle.h"
#include <vector>
#include <cmath>
#include "../common/typedefs.h"


CBottle::CBottle(unsigned int radialSegs, unsigned int heightSegs, GLuint pattern)
    : CShape() {
    generateBottle(radialSegs, heightSegs, pattern);
}

CBottle::~CBottle() {
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
    glDeleteVertexArrays(1, &_vao);
    if (_points) delete[] _points;
    if (_idx)    delete[] _idx;
}

void CBottle::draw() {
    glUseProgram(_shaderProg);
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CBottle::drawRaw() {
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CBottle::update(float dt) {
    // 可加入旋轉或動畫
}

void CBottle::generateBottle(unsigned int radialSegs, unsigned int heightSegs, GLuint pattern) {
    _vtxAttrCount = 11;
    const auto& prof = bottleProfile;
    unsigned int P = (unsigned int)prof.size();

    unsigned int sideVerts = P * (radialSegs + 1);
    unsigned int bottomVerts = 1 + (radialSegs + 1);

    _vtxCount = sideVerts + bottomVerts;
    _idxCount = (P - 1) * radialSegs * 6 + radialSegs * 3;

    _points = new GLfloat[_vtxCount * _vtxAttrCount];
    _idx = new GLuint[_idxCount];

    glm::vec3 baseColor, topColor;
    switch (pattern) {
    case 1:
    default:
        baseColor = glm::vec3(1.0f, 0.85f, 0.7f);
        topColor = glm::vec3(0.9f, 0.7f, 0.5f);
        break;
    case 2:
        baseColor = glm::vec3(0.8f, 0.9f, 1.0f);
        topColor = glm::vec3(0.6f, 0.8f, 0.95f);
        break;
    case 3:
        baseColor = glm::vec3(0.9f, 0.9f, 0.6f);
        topColor = glm::vec3(0.7f, 0.8f, 0.3f);
        break;
    case 4:
        baseColor = glm::vec3(0.85f, 0.75f, 0.85f);
        topColor = glm::vec3(0.65f, 0.5f, 0.85f);
        break;
    }

    unsigned int vi = 0;
    for (unsigned int i = 0; i < P; ++i) {
        float r = prof[i].x;
        float z = prof[i].y;
        float t = z / prof.back().y;
        glm::vec3 color = glm::mix(baseColor, topColor, t);

        for (unsigned int j = 0; j <= radialSegs; ++j) {
            float u = float(j) / radialSegs;
            float th = u * 2.0f * M_PI;
            float x = r * cosf(th);
            float y = r * sinf(th);
            glm::vec3 pos(x, z, -y);  // Y 軸朝上
            glm::vec3 normal = glm::normalize(glm::vec3(cosf(th), 0.0f, -sinf(th)));
            glm::vec2 uv(u, t);

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
    _points[vi * _vtxAttrCount + 0] = 0.0f;
    _points[vi * _vtxAttrCount + 1] = 0.0f;
    _points[vi * _vtxAttrCount + 2] = 0.0f;
    _points[vi * _vtxAttrCount + 3] = baseColor.r;
    _points[vi * _vtxAttrCount + 4] = baseColor.g;
    _points[vi * _vtxAttrCount + 5] = baseColor.b;
    _points[vi * _vtxAttrCount + 6] = 0.0f;
    _points[vi * _vtxAttrCount + 7] = -1.0f;
    _points[vi * _vtxAttrCount + 8] = 0.0f;
    _points[vi * _vtxAttrCount + 9] = 0.5f;
    _points[vi * _vtxAttrCount + 10] = 0.5f;
    ++vi;

    float bottomR = prof.front().x;
    for (unsigned int j = 0; j <= radialSegs; ++j) {
        float u = float(j) / radialSegs;
        float th = u * 2.0f * M_PI;
        float x = bottomR * cosf(th);
        float y = bottomR * sinf(th);
        glm::vec3 color = baseColor;
        _points[vi * _vtxAttrCount + 0] = x;
        _points[vi * _vtxAttrCount + 1] = 0.0f;
        _points[vi * _vtxAttrCount + 2] = -y;
        _points[vi * _vtxAttrCount + 3] = color.r;
        _points[vi * _vtxAttrCount + 4] = color.g;
        _points[vi * _vtxAttrCount + 5] = color.b;
        _points[vi * _vtxAttrCount + 6] = 0.0f;
        _points[vi * _vtxAttrCount + 7] = -1.0f;
        _points[vi * _vtxAttrCount + 8] = 0.0f;
        _points[vi * _vtxAttrCount + 9] = 0.5f + 0.5f * cosf(th);
        _points[vi * _vtxAttrCount + 10] = 0.5f + 0.5f * sinf(th);
        ++vi;
    }

    unsigned int ii = 0;
    for (unsigned int i = 0; i < P - 1; ++i) {
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
