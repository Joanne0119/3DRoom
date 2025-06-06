#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "../common/CMaterial.h"

class CShape
{
public:
	CShape();
	virtual ~CShape();
	virtual void draw();
	virtual void drawRaw(); //不使用 shader
	virtual void reset();
	virtual void update(float dt);
	void setupVertexAttributes();
	void setShaderID(GLuint shaderID, int shadeingmode = 1); // 預設使用傳入的 vertex color
	void setColor(glm::vec4 vColor); // 設定模型的顏色
	void setScale(glm::vec3 vScale); // 設定模型的縮放比
	void setPos(glm::vec3 vPt); // 設定模型的位置
	void setRotate(float angle, const glm::vec3& axis); // 設定模型的旋轉角度與旋轉軸
	void setTransformMatrix(glm::mat4 mxMatrix);
	void updateMatrix();
	glm::vec3 getPos(); // 取得模型的位置
    glm::vec3 getScale();
    glm::vec3 getColor();
	glm::mat4 getModelMatrix();
	glm::mat4 getTransMatrix();
	GLuint getShaderProgram();

	// 材質管理
	void setMaterial(const CMaterial& material);
	void uploadMaterial();

protected:
	int _vtxCount, _vtxAttrCount, _idxCount; // 頂點數, 頂點屬性數,點頂索引值數
	GLfloat* _points;
	GLuint* _idx;
	GLuint _vao, _vbo, _ebo;
	GLuint _shaderProg;
	GLint _modelMxLoc;
	GLint _shadingModeLoc, _uShadingMode; //上色模式的進入點, 上色模式
	GLint _colorLoc; // 上色模式的進入點
	bool _bRotation, _bScale, _bPos, _bObjColor;
	bool _bTransform, _bOnTransform;	
	// _bTransform : true 代表有設定新的轉換矩陣
	// _bOnTransform : true 代表曾經設定過轉換矩陣，用於判斷是否需要更新 model matrix
	glm::vec4 _color; // 使用 RGBA
	glm::vec3 _scale; // 模型的縮放值
	glm::vec3 _pos;  // 模型的位置

	// 設定模型的初始與運動下的參數
	GLfloat   _rotAngle; // 模型的旋轉角度
	glm::vec3 _rotAxis; // 模型的旋轉軸
	glm::mat4 _mxRotation; // 模型目前的旋轉矩陣
	glm::mat4 _mxScale, _mxTrans, _mxTRS; // 模型的縮放、位移與縮放旋轉位移的整合矩陣
	glm::mat4 _mxTransform, _mxFinal; // 額外增加的轉換矩陣與最終的模型矩陣

	// 材質
	CMaterial _material;
};
