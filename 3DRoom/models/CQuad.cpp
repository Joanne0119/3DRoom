#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "CQuad.h"

CQuad::CQuad() : CShape() 
{
	_vtxCount = 4; _vtxAttrCount = 11; _idxCount = 6;

	_points = new GLfloat[_vtxCount * _vtxAttrCount]{
		// 位置            // 顏色         // 法向量       // 貼圖座標
		-0.5f, -0.5f, 0.0f, 0.5f, 0.7f, 0.4f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 左下
		 0.5f, -0.5f, 0.0f, 0.5f, 0.7f, 0.4f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // 右下
		 0.5f,  0.5f, 0.0f, 0.5f, 0.7f, 0.4f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // 右上
		-0.5f,  0.5f, 0.0f, 0.5f, 0.7f, 0.4f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f  // 左上
	};
	_idx = new GLuint[_idxCount]{ 0, 1, 2, 2, 3, 0 };

	//	setupVertexAttributes(); // 這行改在主程式由物件呼叫，就能讓類別可宣告實體物件
}

CQuad::~CQuad()
{
	glDeleteBuffers(1, &_vbo);  //先釋放 VBO 與 EBO
	glDeleteBuffers(1, &_ebo);
	glDeleteVertexArrays(1, &_vao); //再釋放 VAO
	if (_points != NULL) delete[] _points;
	if (_idx != NULL) delete[] _idx;
}

void CQuad::draw()
{
	glUseProgram(_shaderProg);
	glBindVertexArray(_vao);
	updateMatrix();
	glUniform1i(_shadingModeLoc, _uShadingMode);
	if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
	glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void CQuad::drawRaw()
{
	glBindVertexArray(_vao);
	updateMatrix();
	glUniform1i(_shadingModeLoc, _uShadingMode);
	if (_bObjColor) glUniform4fv(_colorLoc, 1, glm::value_ptr(_color));
	glDrawElements(GL_TRIANGLES, _idxCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void CQuad::update(float dt)
{

}

void CQuad::reset() {
	CShape::reset();
	// 如有需要，可加入其他特定於四邊形的重設動作
}
