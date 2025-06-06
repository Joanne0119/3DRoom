#include <glm/gtc/type_ptr.hpp>

#include "CSprite2D.h"
#include "initshader.h"

CSprite2D::CSprite2D()
{
	_vtxCount = _vtxAttrCount = _idxCount = 0;
	_vao = 0; _vbo = 0; _ebo = 0;
	_shaderProg = 0;
	_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	_color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	_rotAngle = 0.0f;
	_rotAxis = glm::vec3(0.0f,1.0f,0.0f);
	_mxScale = glm::mat4(1.0f);
	_mxTrans = glm::mat4(1.0f);
	_mxTransform = glm::mat4(1.0f);
	_mxRotation = glm::mat4(1.0f);
	_mxTRS = glm::mat4(1.0f);
	_mxFinal = glm::mat4(1.0f);
	_colorLoc = _modelMxLoc = 0;
	_bRotation = _bScale = _bPos = _bTransform = _bOnTransform = false;
	_coloringMode = 2;  // 預設上色模式，1 : vertex color, 2: uniform color(object color)
	_bObjColor = false; // 預設不使用物件顏色
	_coloringModeLoc = 0; _coloringMode = 2; //上色模式的進入點, 上色模式
	_colorLoc = 0; // 上色模式的進入點
}

CSprite2D::~CSprite2D()
{

}

void CSprite2D::setupVertexAttributes()
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

	//貼圖座標屬性
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, _vtxAttrCount * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0); // 解除對 VAO 的綁定
}

void CSprite2D::setShaderID(GLuint shaderID)
{
	_shaderProg = shaderID;
	glUseProgram(_shaderProg);
	_modelMxLoc = glGetUniformLocation(_shaderProg, "mxModel"); 	// 取得 mxModel 變數的位置
	glUniformMatrix4fv(_modelMxLoc, 1, GL_FALSE, glm::value_ptr(_mxTRS));
	_coloringModeLoc = glGetUniformLocation(_shaderProg, "iColorType"); 	// 取得 iColorType 變數的位置
	glUniform1i(_coloringModeLoc, _coloringMode);
}

void CSprite2D::setColor(glm::vec4 vColor)
{
	// 透過 glUniform 傳入模型的顏色
	_color = vColor;
	_colorLoc = glGetUniformLocation(_shaderProg, "ui4Color"); 	// 取得 ui4Color 變數的位置
	glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
	_coloringMode = 2; // 設定上色模式為 uniform color
	glUniform1i(_coloringModeLoc, _coloringMode);
	_bObjColor = true;
}

void CSprite2D::draw() {
	//glUseProgram(_shaderProg);
	//updateMatrix();
	//glBindVertexArray(_vao);
	// 預設不呼叫實際的繪製指令，由子類別決定是否直接呼叫 glDrawElements()
}

void CSprite2D::drawRaw()
{

}

void CSprite2D::update(float dt)
{


}

void CSprite2D::setScale(glm::vec3 vScale)
{
	_scale = vScale;
	_bScale = true;
	_mxScale = glm::scale(glm::mat4(1.0f), _scale);
}

void CSprite2D::setPos(glm::vec3 vPt)
{
	_pos = vPt;
	_bPos = true;
	_mxTrans = glm::translate(glm::mat4(1.0f), _pos);
}

glm::vec3 CSprite2D::getPos() { return _pos; }
glm::mat4 CSprite2D::getTransMatrix() { return _mxTrans; }

void CSprite2D::setRotate(float angle, const glm::vec3& axis)
{
	float rad = glm::radians(angle); 	// glm::angleAxis 需要弧度
	_mxRotation = glm::rotate(glm::mat4(1.0f), rad, glm::normalize(axis)); // 計算旋轉軸下旋轉角度所對應旋轉矩陣
	_bRotation = true;
}

void CSprite2D::setTransformMatrix(glm::mat4 mxMatrix)
{
	_bOnTransform = _bTransform = true;
	_mxTransform = mxMatrix;
}

void CSprite2D::updateMatrix()
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

glm::mat4 CSprite2D::getModelMatrix() { return _mxFinal; }

GLuint CSprite2D::getShaderProgram() { return _shaderProg; }

void CSprite2D::reset()
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
	_coloringMode = 1; // 預設上色模式，1 : vertex color, 2: uniform color(object color)
	_bObjColor = false; // 預設不使用物件顏色
}