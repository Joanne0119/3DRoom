#include <glm/gtc/type_ptr.hpp>

#include "CShape.h"
#include "../common/typedefs.h"

CShape::CShape()
{
	_vtxCount = _vtxAttrCount = _idxCount = 0;
	_vao = 0; _vbo = 0; _ebo = 0;
	_shaderProg = 0;
	_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	_color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	_rotAngle = 0.0f;
	_rotAxis = glm::vec3(0.0f,1.0f,0.0f);
	_bRotation = _bScale = _bPos = _bTransform = _bOnTransform = false;
	_mxScale = glm::mat4(1.0f);
	_mxTrans = glm::mat4(1.0f);
	_mxRotation = glm::mat4(1.0f);
	_mxTRS = glm::mat4(1.0f);
	_mxTransform = glm::mat4(1.0f);
	_mxFinal = glm::mat4(1.0f);
	_colorLoc = _modelMxLoc = 0;
	_points = nullptr; _idx = nullptr;
	_uShadingMode = 1; // 預設上色模式，1 : vertex color, 2: uniform color(object color)
	_bObjColor = false; // 預設不使用物件顏色
	_shadingModeLoc = 0; // 上色模式的進入點
}

CShape::~CShape()
{

}

void CShape::setupVertexAttributes()
{
	// 設定 VAO、VBO 與 EBO
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ebo);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(_vao);

	// 設定 VBO
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _vtxCount * _vtxAttrCount * sizeof(float), _points, GL_STATIC_DRAW);

	// 設定 EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _idxCount * sizeof(GLuint), _idx, GL_STATIC_DRAW);

	// 位置屬性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, _vtxAttrCount * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	// 顏色屬性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, _vtxAttrCount * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//法向量屬性
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, _vtxAttrCount * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//貼圖座標屬性
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, _vtxAttrCount * sizeof(float), BUFFER_OFFSET(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glBindVertexArray(0); // 解除對 VAO 的綁定
}

void CShape::setShaderID(GLuint shaderID, int shadeingmode)
{
	_shaderProg = shaderID;
	_uShadingMode = shadeingmode;
	glUseProgram(_shaderProg);
	_modelMxLoc = glGetUniformLocation(_shaderProg, "mxModel"); 	// 取得 mxModel 變數的位置
	glUniformMatrix4fv(_modelMxLoc, 1, GL_FALSE, glm::value_ptr(_mxTRS));
	_shadingModeLoc = glGetUniformLocation(_shaderProg, "uShadingMode"); 	// 取得 iColorType 變數的位置
	glUniform1i(_shadingModeLoc, _uShadingMode);
}

void CShape::setColor(glm::vec4 vColor)
{
	// 透過 glUniform 傳入模型的顏色
	_color = vColor;
	_colorLoc = glGetUniformLocation(_shaderProg, "ui4Color"); 	// 取得 ui4Color 變數的位置
	glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
	_uShadingMode = 2; // 設定上色模式為 uniform color
	glUniform1i(_shadingModeLoc, _uShadingMode);
	_bObjColor = true;
}

void CShape::draw() {
	//glUseProgram(_shaderProg);
	//updateMatrix();
	//glBindVertexArray(_vao);
	// 預設不呼叫實際的繪製指令，由子類別決定是否直接呼叫 glDrawElements()
}

void CShape::drawRaw()
{

}

void CShape::update(float dt)
{


}

void CShape::setScale(glm::vec3 vScale)
{
	_scale = vScale;
	_bScale = true;
	_mxScale = glm::scale(glm::mat4(1.0f), _scale);
}

void CShape::setPos(glm::vec3 vPt)
{
	_pos = vPt;
	_bPos = true;
	_mxTrans = glm::translate(glm::mat4(1.0f), _pos);
}

glm::vec3 CShape::getPos() { return _pos; }
glm::mat4 CShape::getTransMatrix() { return _mxTrans; }


void CShape::setRotate(float angle, const glm::vec3& axis)
{
	float rad = glm::radians(angle); 	// glm::angleAxis 需要弧度
	_mxRotation = glm::rotate(glm::mat4(1.0f), rad, glm::normalize(axis)); // 計算旋轉軸下旋轉角度所對應旋轉矩陣
	_bRotation = true;
}

void CShape::updateMatrix()
{
	if (_bScale || _bPos || _bRotation )
	{
		_mxTRS = _mxTrans * _mxRotation * _mxScale;
		if (_bOnTransform == true) _mxFinal = _mxTransform * _mxTRS;
		else _mxFinal = _mxTRS;
		_bScale = _bPos = _bRotation = false;
	}
	if (_bTransform) {
		_mxFinal = _mxTransform * _mxTRS;
		_bTransform = false;
	}
	// 如多個模型使用相同的 shader program,因每一個模型的 mxTRS 都不同，所以每個frame都要更新
	glUniformMatrix4fv(_modelMxLoc, 1, GL_FALSE, glm::value_ptr(_mxFinal));
}

void CShape::setTransformMatrix(glm::mat4 mxMatrix)
{
	_bOnTransform = _bTransform = true;
	_mxTransform = mxMatrix;
}

glm::mat4 CShape::getModelMatrix() { return _mxFinal; }
GLuint CShape::getShaderProgram() { return _shaderProg; }
glm::vec3 CShape::getScale(){ return _scale; }
glm::vec3 CShape::getColor(){ return _color; }

void CShape::reset()
{
	_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	_color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	_rotAngle = 0.0f;
	_rotAxis = glm::vec3(0.0f, 1.0f, 0.0f);

	_bRotation = _bScale = _bPos = _bTransform = _bOnTransform = false;
	_mxScale = glm::mat4(1.0f);
	_mxTrans = glm::mat4(1.0f);
	_mxTRS = glm::mat4(1.0f);
	_mxRotation = glm::mat4(1.0f);
	_mxTransform = glm::mat4(1.0f);
	_mxFinal = glm::mat4(1.0f);
	_uShadingMode = 1; // 預設上色模式，1 : vertex color, 2: uniform color(object color)
	_bObjColor = false; // 預設不使用物件顏色
}


void CShape::setMaterial(const CMaterial& material) {
	_material = material;
}

void CShape::uploadMaterial()  {
	_material.uploadToShader(_shaderProg, "uMaterial");
}
