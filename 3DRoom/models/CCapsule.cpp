// CCapsule.cpp
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <cmath>

#include "CCapsule.h"
#include "../common/typedefs.h"

CCapsule::CCapsule(float radius, float height, GLuint sectors, GLuint stacks, GLuint pattern)
    : CShape()
{
    generateCapsule(radius, height, sectors, stacks, pattern);
}

CCapsule::~CCapsule() {
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
    glDeleteVertexArrays(1, &_vao);
    if (_points) delete[] _points;
    if (_idx)    delete[] _idx;
}

void CCapsule::draw() {
    glUseProgram(_shaderProg);
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CCapsule::drawRaw() {
    updateMatrix();
    glBindVertexArray(_vao);
    glUniform1i(_shadingModeLoc, _uShadingMode);
    if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
    glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CCapsule::reset() {
    CShape::reset();
}

void CCapsule::update(float dt) {
    // 可加入動畫或物理行為
}
void CCapsule::generateCapsule(float radius,
    float height,
    GLuint sectors,
    GLuint stacks,
    GLuint pattern)
{
    // 類似 CDonut / CSphere 的 pattern 用法 :contentReference[oaicite:0]{index=0}
    glm::vec4 bottomColor, topColor;
    switch (pattern) {
    case 1:
        bottomColor = glm::vec4(1.00f, 0.78f, 0.55f, 1.0f);
        topColor = glm::vec4(1.00f, 0.88f, 0.70f, 1.0f);
        break;
    case 2:
        bottomColor = glm::vec4(0.85f, 0.70f, 0.55f, 1.0f);
        topColor = glm::vec4(0.95f, 0.85f, 0.75f, 1.0f);
        break;
    case 3:
        bottomColor = glm::vec4(0.55f, 0.40f, 0.65f, 1.0f);
        topColor = glm::vec4(0.75f, 0.60f, 0.85f, 1.0f);
        break;
    case 4:
        bottomColor = glm::vec4(0.45f, 0.60f, 0.75f, 1.0f);
        topColor = glm::vec4(0.65f, 0.80f, 0.90f, 1.0f);
        break;
    default:
        bottomColor = glm::vec4(1.00f, 0.78f, 0.55f, 1.0f);
        topColor = glm::vec4(1.00f, 0.88f, 0.70f, 1.0f);
    }

    // 分段計算
    float sectorStep = 2.0f * M_PI / sectors;
    GLuint hemiVerts = (stacks + 1) * (sectors + 1);
    GLuint cylVerts = 2 * (sectors + 1); // 只有上下兩圈
    _vtxAttrCount = 11;
    _vtxCount = hemiVerts * 2 + cylVerts;
    _idxCount = (stacks * sectors + sectors + stacks * sectors) * 6;

    _points = new GLfloat[_vtxCount * _vtxAttrCount];
    _idx = new GLuint[_idxCount];

    GLuint vi = 0, ii = 0;

    // 1) 下半球 (phi: -π/2 → 0)
    float centerZ_bot = -height * 0.5f;
    for (GLuint i = 0; i <= stacks; ++i) {
        float phi = -M_PI_2 + (float)i * (M_PI_2 / stacks);
        float sinP = sinf(phi), cosP = cosf(phi);
        float z = centerZ_bot + radius * sinP;
        float t = (z + (height * 0.5f + radius)) / (height + 2 * radius);
        glm::vec4 color = bottomColor * (1.0f - t) + topColor * t;

        for (GLuint j = 0; j <= sectors; ++j) {
            float theta = j * sectorStep;
            float x = radius * cosP * cosf(theta);
            float y = radius * cosP * sinf(theta);
            glm::vec3 pos = glm::vec3(x, z, -y);
            glm::vec3 normal = glm::normalize(glm::vec3(cosP * cosf(theta), sinP, -cosP * sinf(theta)));
            glm::vec2 tex = glm::vec2(float(j) / sectors, float(i) / stacks * 0.5f);

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

    // 2) 圓柱 (z: -h/2 → +h/2, 只有上下兩圈)
    for (GLuint ring = 0; ring < 2; ++ring) {
        float z = (ring == 0 ? -height * 0.5f : height * 0.5f);
        float t = (z + (height * 0.5f + radius)) / (height + 2 * radius);
        glm::vec4 color = bottomColor * (1.0f - t) + topColor * t;

        for (GLuint j = 0; j <= sectors; ++j) {
            float theta = j * sectorStep;
            float x = radius * cosf(theta);
            float y = radius * sinf(theta);
            glm::vec3 pos = glm::vec3(x, z, -y);
            glm::vec3 normal = glm::normalize(glm::vec3(cosf(theta), 0.0f, -sinf(theta)));
            glm::vec2 tex = glm::vec2(float(j) / sectors, 0.5f + ring * 0.5f);

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

    // 3) 上半球 (phi: 0 → π/2)
    float centerZ_top = height * 0.5f;
    for (GLuint i = 0; i <= stacks; ++i) {
        float phi = (float)i * (M_PI_2 / stacks);
        float sinP = sinf(phi), cosP = cosf(phi);
        float z = centerZ_top + radius * sinP;
        float t = (z + (height * 0.5f + radius)) / (height + 2 * radius);
        glm::vec4 color = bottomColor * (1.0f - t) + topColor * t;

        for (GLuint j = 0; j <= sectors; ++j) {
            float theta = j * sectorStep;
            float x = radius * cosP * cosf(theta);
            float y = radius * cosP * sinf(theta);
            glm::vec3 pos = glm::vec3(x, z, -y);
            glm::vec3 normal = glm::normalize(glm::vec3(cosP * cosf(theta), sinP, -cosP * sinf(theta)));
            glm::vec2 tex = glm::vec2(float(j) / sectors, 0.5f + 0.5f + float(i) / stacks * 0.5f);

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

    // 填三段索引：下半球、圓柱、上半球
    auto sectionVerts = sectors + 1;
    auto addSection = [&](GLuint start, GLuint segs, GLuint rows) {
        for (GLuint i = 0; i < rows; ++i) {
            for (GLuint j = 0; j < segs; ++j) {
                GLuint cur = start + i * sectionVerts + j;
                GLuint next = cur + sectionVerts;
                _idx[ii++] = cur;      _idx[ii++] = next;      _idx[ii++] = cur + 1;
                _idx[ii++] = cur + 1;    _idx[ii++] = next;      _idx[ii++] = next + 1;
            }
        }
        };
    // 下半球
    addSection(0, sectors, stacks);
    // 圓柱 (rows=1)
    addSection(hemiVerts, sectors, 1);
    // 上半球
    addSection(hemiVerts + 2 * sectionVerts, sectors, stacks);
}


//
//void CCapsule::generateCapsule(float radius,
//    float height,
//    GLuint sectors,
//    GLuint stacks,
//    GLuint pattern)
//{
//    // 類似 CDonut / CSphere 的 pattern 用法 :contentReference[oaicite:0]{index=0}
//    glm::vec4 bottomColor, topColor;
//    switch (pattern) {
//    case 1:
//        bottomColor = glm::vec4(1.00f, 0.78f, 0.55f, 1.0f);
//        topColor = glm::vec4(1.00f, 0.88f, 0.70f, 1.0f);
//        break;
//    case 2:
//        bottomColor = glm::vec4(0.85f, 0.70f, 0.55f, 1.0f);
//        topColor = glm::vec4(0.95f, 0.85f, 0.75f, 1.0f);
//        break;
//    case 3:
//        bottomColor = glm::vec4(0.55f, 0.40f, 0.65f, 1.0f);
//        topColor = glm::vec4(0.75f, 0.60f, 0.85f, 1.0f);
//        break;
//    case 4:
//        bottomColor = glm::vec4(0.45f, 0.60f, 0.75f, 1.0f);
//        topColor = glm::vec4(0.65f, 0.80f, 0.90f, 1.0f);
//        break;
//    default:
//        bottomColor = glm::vec4(1.00f, 0.78f, 0.55f, 1.0f);
//        topColor = glm::vec4(1.00f, 0.88f, 0.70f, 1.0f);
//    }
//
//    // 分段計算
//    float sectorStep = 2.0f * M_PI / sectors;
//    GLuint hemiVerts = (stacks + 1) * (sectors + 1);
//    GLuint cylVerts = 2 * (sectors + 1); // 只有上下兩圈
//    _vtxAttrCount = 11;
//    _vtxCount = hemiVerts * 2 + cylVerts;
//    _idxCount = (stacks * sectors + sectors + stacks * sectors) * 6;
//
//    _points = new GLfloat[_vtxCount * _vtxAttrCount];
//    _idx = new GLuint[_idxCount];
//
//    GLuint vi = 0, ii = 0;
//
//    // 1) 下半球 (phi: -π/2 → 0)
//    float centerZ_bot = -height * 0.5f;
//    for (GLuint i = 0; i <= stacks; ++i) {
//        float phi = -M_PI_2 + (float)i * (M_PI_2 / stacks);
//        float sinP = sinf(phi), cosP = cosf(phi);
//        float z = centerZ_bot + radius * sinP;
//        float t = (z + (height * 0.5f + radius)) / (height + 2 * radius);
//        glm::vec4 color = bottomColor * (1.0f - t) + topColor * t;
//
//        for (GLuint j = 0; j <= sectors; ++j) {
//            float theta = j * sectorStep;
//            float x = radius * cosP * cosf(theta);
//            float y = radius * cosP * sinf(theta);
//            glm::vec3 pos = glm::vec3(x, y, z);
//            glm::vec3 normal = glm::normalize(glm::vec3(cosP * cosf(theta), cosP * sinf(theta), sinP));
//            glm::vec2 tex = glm::vec2(float(j) / sectors, float(i) / stacks * 0.5f);
//
//            _points[vi * _vtxAttrCount + 0] = pos.x;
//            _points[vi * _vtxAttrCount + 1] = pos.y;
//            _points[vi * _vtxAttrCount + 2] = pos.z;
//            _points[vi * _vtxAttrCount + 3] = color.r;
//            _points[vi * _vtxAttrCount + 4] = color.g;
//            _points[vi * _vtxAttrCount + 5] = color.b;
//            _points[vi * _vtxAttrCount + 6] = normal.x;
//            _points[vi * _vtxAttrCount + 7] = normal.y;
//            _points[vi * _vtxAttrCount + 8] = normal.z;
//            _points[vi * _vtxAttrCount + 9] = tex.x;
//            _points[vi * _vtxAttrCount + 10] = tex.y;
//            ++vi;
//        }
//    }
//
//    // 2) 圓柱 (z: -h/2 → +h/2, 只有上下兩圈)
//    for (GLuint ring = 0; ring < 2; ++ring) {
//        float z = (ring == 0 ? -height * 0.5f : height * 0.5f);
//        float t = (z + (height * 0.5f + radius)) / (height + 2 * radius);
//        glm::vec4 color = bottomColor * (1.0f - t) + topColor * t;
//
//        for (GLuint j = 0; j <= sectors; ++j) {
//            float theta = j * sectorStep;
//            float x = radius * cosf(theta);
//            float y = radius * sinf(theta);
//            glm::vec3 pos = glm::vec3(x, y, z);
//            glm::vec3 normal = glm::normalize(glm::vec3(cosf(theta), sinf(theta), 0.0f));
//            glm::vec2 tex = glm::vec2(float(j) / sectors, 0.5f + ring * 0.5f);
//
//            _points[vi * _vtxAttrCount + 0] = pos.x;
//            _points[vi * _vtxAttrCount + 1] = pos.y;
//            _points[vi * _vtxAttrCount + 2] = pos.z;
//            _points[vi * _vtxAttrCount + 3] = color.r;
//            _points[vi * _vtxAttrCount + 4] = color.g;
//            _points[vi * _vtxAttrCount + 5] = color.b;
//            _points[vi * _vtxAttrCount + 6] = normal.x;
//            _points[vi * _vtxAttrCount + 7] = normal.y;
//            _points[vi * _vtxAttrCount + 8] = normal.z;
//            _points[vi * _vtxAttrCount + 9] = tex.x;
//            _points[vi * _vtxAttrCount + 10] = tex.y;
//            ++vi;
//        }
//    }
//
//    // 3) 上半球 (phi: 0 → π/2)
//    float centerZ_top = height * 0.5f;
//    for (GLuint i = 0; i <= stacks; ++i) {
//        float phi = (float)i * (M_PI_2 / stacks);
//        float sinP = sinf(phi), cosP = cosf(phi);
//        float z = centerZ_top + radius * sinP;
//        float t = (z + (height * 0.5f + radius)) / (height + 2 * radius);
//        glm::vec4 color = bottomColor * (1.0f - t) + topColor * t;
//
//        for (GLuint j = 0; j <= sectors; ++j) {
//            float theta = j * sectorStep;
//            float x = radius * cosP * cosf(theta);
//            float y = radius * cosP * sinf(theta);
//            glm::vec3 pos = glm::vec3(x, y, z);
//            glm::vec3 normal = glm::normalize(glm::vec3(cosP * cosf(theta), cosP * sinf(theta), sinP));
//            glm::vec2 tex = glm::vec2(float(j) / sectors, 0.5f + 0.5f + float(i) / stacks * 0.5f);
//
//            _points[vi * _vtxAttrCount + 0] = pos.x;
//            _points[vi * _vtxAttrCount + 1] = pos.y;
//            _points[vi * _vtxAttrCount + 2] = pos.z;
//            _points[vi * _vtxAttrCount + 3] = color.r;
//            _points[vi * _vtxAttrCount + 4] = color.g;
//            _points[vi * _vtxAttrCount + 5] = color.b;
//            _points[vi * _vtxAttrCount + 6] = normal.x;
//            _points[vi * _vtxAttrCount + 7] = normal.y;
//            _points[vi * _vtxAttrCount + 8] = normal.z;
//            _points[vi * _vtxAttrCount + 9] = tex.x;
//            _points[vi * _vtxAttrCount + 10] = tex.y;
//            ++vi;
//        }
//    }
//
//    // 填三段索引：下半球、圓柱、上半球
//    auto sectionVerts = sectors + 1;
//    auto addSection = [&](GLuint start, GLuint segs, GLuint rows) {
//        for (GLuint i = 0; i < rows; ++i) {
//            for (GLuint j = 0; j < segs; ++j) {
//                GLuint cur = start + i * sectionVerts + j;
//                GLuint next = cur + sectionVerts;
//                _idx[ii++] = cur;      _idx[ii++] = next;      _idx[ii++] = cur + 1;
//                _idx[ii++] = cur + 1;    _idx[ii++] = next;      _idx[ii++] = next + 1;
//            }
//        }
//        };
//    // 下半球
//    addSection(0, sectors, stacks);
//    // 圓柱 (rows=1)
//    addSection(hemiVerts, sectors, 1);
//    // 上半球
//    addSection(hemiVerts + 2 * sectionVerts, sectors, stacks);
//
//    // 建立 VAO/VBO/EBO
//    //setupVertexAttributes();
//}
//
//
