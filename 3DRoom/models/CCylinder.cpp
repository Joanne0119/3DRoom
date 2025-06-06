// CCylinder.cpp
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include "CCylinder.h"
#include "../common/typedefs.h"

// Constructor / Destructor
CCylinder::CCylinder(GLuint sectors, GLuint pattern) : CShape() {
    generateCylinder(sectors, pattern);
}

CCylinder::~CCylinder() {
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
    glDeleteVertexArrays(1, &_vao);
    if (_points) delete[] _points;
    if (_idx)    delete[] _idx;
}

// Draw methods
void CCylinder::draw() {
    glUseProgram(_shaderProg);
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CCylinder::drawRaw() {
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CCylinder::reset() {
    CShape::reset();
}

void CCylinder::update(float dt) {
    // 可以加入動畫邏輯
}

void CCylinder::generateCylinder(GLuint sectors, GLuint pattern) {
    const float radius = 1.0f;
    const float height = 1.0f;
    const GLuint stacks = 1;  // 只有上下兩層

    // 正確計算頂點數：
    //  1) 側面：(stacks+1)*(sectors+1)
    //  2) 底部中心 + 底部扇形：(1 + (sectors+1))
    //  3) 頂部中心 + 頂部扇形：(1 + (sectors+1))
    _vtxCount = (stacks + 1) * (sectors + 1) + 2 * (sectors + 1) + 2;
    _idxCount = sectors * 6 + sectors * 3 + sectors * 3;
    _vtxAttrCount = 11;

    _points = new GLfloat[_vtxCount * _vtxAttrCount];
    _idx = new GLuint[_idxCount];

    glm::vec4 topColor, bottomColor;
    switch (pattern) {
    case 1: default:
        topColor = { 0.65f, 0.80f, 0.90f, 1.0f };
        bottomColor = { 0.45f, 0.60f, 0.75f, 1.0f }; break;
    case 2:
        topColor = { 0.75f, 0.60f, 0.85f, 1.0f };
        bottomColor = { 0.55f, 0.40f, 0.65f, 1.0f }; break;
    case 3:
        topColor = { 0.95f, 0.85f, 0.75f, 1.0f };
        bottomColor = { 0.85f, 0.70f, 0.55f, 1.0f }; break;
    case 4:
        topColor = { 1.00f, 0.88f, 0.70f, 1.0f };
        bottomColor = { 1.00f, 0.78f, 0.55f, 1.0f }; break;
    }

    float sectorStep = 2.0f * M_PI / sectors;
    GLuint vi = 0, ii = 0;

    // 側面頂點
    for (GLuint i = 0; i <= stacks; ++i) {
        float y = (i == 0 ? 0.0f : height);
        float v = float(i) / stacks;
        glm::vec4 color = glm::mix(bottomColor, topColor, v);
        for (GLuint j = 0; j <= sectors; ++j) {
            float theta = j * sectorStep;
            float x = cosf(theta) * radius;
            float z = -sinf(theta) * radius;
            glm::vec3 pos(x, y, z);
            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
            glm::vec2 tex = glm::vec2(float(j) / sectors, v);

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

    for (GLuint i = 0; i < stacks; ++i) {
        for (GLuint j = 0; j < sectors; ++j) {
            GLuint cur = i * (sectors + 1) + j;
            GLuint next = cur + (sectors + 1);
            _idx[ii++] = cur; _idx[ii++] = next; _idx[ii++] = cur + 1;
            _idx[ii++] = cur + 1; _idx[ii++] = next; _idx[ii++] = next + 1;
        }
    }

    // bottom center
    GLuint baseCenter = vi++;
    _points[baseCenter * _vtxAttrCount + 0] = 0.0f;
    _points[baseCenter * _vtxAttrCount + 1] = 0.0f;
    _points[baseCenter * _vtxAttrCount + 2] = 0.0f;
    _points[baseCenter * _vtxAttrCount + 3] = bottomColor.r;
    _points[baseCenter * _vtxAttrCount + 4] = bottomColor.g;
    _points[baseCenter * _vtxAttrCount + 5] = bottomColor.b;
    _points[baseCenter * _vtxAttrCount + 6] = 0.0f;
    _points[baseCenter * _vtxAttrCount + 7] = -1.0f;
    _points[baseCenter * _vtxAttrCount + 8] = 0.0f;
    _points[baseCenter * _vtxAttrCount + 9] = 0.5f;
    _points[baseCenter * _vtxAttrCount + 10] = 0.5f;

    for (GLuint j = 0; j <= sectors; ++j) {
        float theta = j * sectorStep;
        float x = cosf(theta) * radius;
        float z = -sinf(theta) * radius;
        _points[vi * _vtxAttrCount + 0] = x;
        _points[vi * _vtxAttrCount + 1] = 0.0f;
        _points[vi * _vtxAttrCount + 2] = z;
        _points[vi * _vtxAttrCount + 3] = bottomColor.r;
        _points[vi * _vtxAttrCount + 4] = bottomColor.g;
        _points[vi * _vtxAttrCount + 5] = bottomColor.b;
        _points[vi * _vtxAttrCount + 6] = 0.0f;
        _points[vi * _vtxAttrCount + 7] = -1.0f;
        _points[vi * _vtxAttrCount + 8] = 0.0f;
        _points[vi * _vtxAttrCount + 9] = (x + 1.0f) * 0.5f;
        _points[vi * _vtxAttrCount + 10] = (z + 1.0f) * 0.5f;
        ++vi;
    }
    for (GLuint j = 0; j < sectors; ++j) {
        _idx[ii++] = baseCenter;
        _idx[ii++] = baseCenter + j + 2;
        _idx[ii++] = baseCenter + j + 1;
    }

    // top center
    GLuint topCenter = vi++;
    _points[topCenter * _vtxAttrCount + 0] = 0.0f;
    _points[topCenter * _vtxAttrCount + 1] = height;
    _points[topCenter * _vtxAttrCount + 2] = 0.0f;
    _points[topCenter * _vtxAttrCount + 3] = topColor.r;
    _points[topCenter * _vtxAttrCount + 4] = topColor.g;
    _points[topCenter * _vtxAttrCount + 5] = topColor.b;
    _points[topCenter * _vtxAttrCount + 6] = 0.0f;
    _points[topCenter * _vtxAttrCount + 7] = 1.0f;
    _points[topCenter * _vtxAttrCount + 8] = 0.0f;
    _points[topCenter * _vtxAttrCount + 9] = 0.5f;
    _points[topCenter * _vtxAttrCount + 10] = 0.5f;

    for (GLuint j = 0; j <= sectors; ++j) {
        float theta = j * sectorStep;
        float x = cosf(theta) * radius;
        float z = -sinf(theta) * radius;
        _points[vi * _vtxAttrCount + 0] = x;
        _points[vi * _vtxAttrCount + 1] = height;
        _points[vi * _vtxAttrCount + 2] = z;
        _points[vi * _vtxAttrCount + 3] = topColor.r;
        _points[vi * _vtxAttrCount + 4] = topColor.g;
        _points[vi * _vtxAttrCount + 5] = topColor.b;
        _points[vi * _vtxAttrCount + 6] = 0.0f;
        _points[vi * _vtxAttrCount + 7] = 1.0f;
        _points[vi * _vtxAttrCount + 8] = 0.0f;
        _points[vi * _vtxAttrCount + 9] = (x + 1.0f) * 0.5f;
        _points[vi * _vtxAttrCount + 10] = (z + 1.0f) * 0.5f;
        ++vi;
    }
    for (GLuint j = 0; j < sectors; ++j) {
        _idx[ii++] = topCenter;
        _idx[ii++] = topCenter + j + 1;
        _idx[ii++] = topCenter + j + 2;
    }
}
