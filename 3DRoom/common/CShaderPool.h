#pragma once
#include <string>
#include <vector>
#include <GL/glew.h>
#include "initshader.h"

// 儲存 shader 資訊的結構
struct ShaderEntry {
    std::string vertexShaderName;
    std::string fragmentShaderName;
    GLuint shaderID;
};

class CShaderPool {
public:
    // 取得全域唯一的 CShaderPool 實例 (Singleton)
    static CShaderPool& getInstance();

    // 傳入 vertex 與 fragment shader 的名稱，若已建立則回傳對應 shaderID，
    // 否則呼叫外部 createShader 建立新 shader，再回傳 shaderID
    GLuint getShader(const std::string& vertexShaderName, const std::string& fragmentShaderName);

private:
    // 私有建構子與解構子
    CShaderPool();
    ~CShaderPool();

    // 禁止拷貝與指派操作
    CShaderPool(const CShaderPool&) = delete;
    CShaderPool& operator=(const CShaderPool&) = delete;

    // 使用 vector 儲存所有 shader 的資料
    std::vector<ShaderEntry> m_shaderEntries;
};
