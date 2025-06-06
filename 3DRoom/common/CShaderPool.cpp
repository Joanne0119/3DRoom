#pragma once
#include "CShaderPool.h"
#include <iostream>

CShaderPool& CShaderPool::getInstance() {
    static CShaderPool instance;
    return instance;
}

CShaderPool::CShaderPool() {
    // 可在此做其他初始化工作
}

CShaderPool::~CShaderPool() {
    // 呼叫 glDeleteProgram 釋放 shader 資源
     for (const auto& entry : m_shaderEntries) { glDeleteProgram(entry.shaderID); }
}

GLuint CShaderPool::getShader(const std::string& vertexShaderName, const std::string& fragmentShaderName) {
    // 先檢查 vector 中是否已存在相同 shader 資訊
    for (const auto& entry : m_shaderEntries) {
        if (entry.vertexShaderName == vertexShaderName && entry.fragmentShaderName == fragmentShaderName) {
            return entry.shaderID;
        }
    }

    // 不存在時，呼叫外部函式建立新的 shader
    GLuint shaderID = createShader(vertexShaderName, fragmentShaderName);

    // 儲存新的 shader 資訊到 vector 中
    ShaderEntry newEntry = { vertexShaderName, fragmentShaderName, shaderID };
    m_shaderEntries.push_back(newEntry);

    return shaderID;
}