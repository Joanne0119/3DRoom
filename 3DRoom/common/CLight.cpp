// CLight.cpp

#include "CLight.h"
#include <glm/gtc/type_ptr.hpp>
#include "typedefs.h"
#include <iostream>

CLight::CLight(
    glm::vec3 position, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular,
    float constant, float linear, float quadratic) {
    _position = position; 
    _ambient = ambient;  _diffuse = diffuse;  _specular = specular;
    _ambient.w = 1.0f; _diffuse.w = 1.0f; _specular.w = 1.0f;
    _constant = constant; _linear = linear; _quadratic = quadratic;
    _intensity = 1.0f;
    _type = LightType::POINT; _posStart = position;
    _direction = glm::vec3(1.0f); _target = glm::vec3(1.0f);
    _innerCutOff = 0.0f; _outerCutOff = 0.0f; _exponent = 1.0f;
    _displayOn = true; _motionOn = false; _clock = 0.0f;
    _lighingOn = true; _lightObj.setPos(position);
}
//CLight::CLight(glm::vec3 position, glm::vec3 direction, float innerCutOffDeg, float outerCutOffDeg,
//    glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic) {
//    _position = position; _direction = direction; _enabled = true;
//    _innerCutOff = innerCutOffDeg; _outerCutOff = outerCutOffDeg;
//    _ambient = ambient;   _diffuse = diffuse; _specular = specular;
//    _constant = constant; _linear = linear;   _quadratic = quadratic;
//}
CLight::CLight(glm::vec3 position, glm::vec3 target, float innerCutOffDeg, float outerCutOffDeg, float exponent,
    glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float constant, float linear, float quadratic) {
    _position = position; _target = target; _direction = glm::normalize(target - position);
    _intensity = 1.0f;  _exponent = exponent;
    _innerCutOff = glm::cos(glm::radians(innerCutOffDeg)); 
    _outerCutOff = glm::cos(glm::radians(outerCutOffDeg));
    _ambient = ambient;  _diffuse = diffuse;  _specular = specular;
    _ambient.w = 1.0f; _diffuse.w = 1.0f; _specular.w = 1.0f;
    _constant = constant; _linear = linear; _quadratic = quadratic;
    _type = LightType::SPOT; _posStart = position;
    _displayOn = true; _motionOn = false; _clock = 0.0f;
    _lighingOn = true; _lightObj.setPos(position);
}

CLight::~CLight() = default;


void CLight::setPos(glm::vec3 pos) { 
    _position = pos; 
    
    if ( _type == LightType::SPOT ) {
        _direction = glm::normalize(_target - _position);
    }
}
glm::vec3 CLight::getPos()  { return _position; }

void CLight::setAmbient( glm::vec4 amb) { _ambient = amb; }
glm::vec4 CLight::getAmbient()  { return _ambient; }

void CLight::setDiffuse( glm::vec4 diff) { _diffuse = diff; }
glm::vec4 CLight::getDiffuse()  { return _diffuse; }

void CLight::setSpecular( glm::vec4 spec) { _specular = spec; }
glm::vec4 CLight::getSpecular()  { return _specular; }

void CLight::setIntensity(float intensity) { 
    _intensity = intensity;
    _ambient  = _ambient  * _intensity;
    _diffuse  = _diffuse  * _intensity;
    _specular = _specular * _intensity;
    _ambient.w = 1.0f; _diffuse.w = 1.0f; _specular.w = 1.0f;
}

void CLight::setAttenuation(float c, float l, float q) {
    _constant = c; _linear = l;  _quadratic = q;
}
void CLight::getAttenuation(float& c, float& l, float& q) {
    c = _constant; l = _linear;  q = _quadratic; 
}

void CLight::setLightOn(bool enable) { _lighingOn = enable; }
bool CLight::isLightOn(){ return _lighingOn; }

void CLight::setMotionEnabled() { _motionOn = !_motionOn; }


void CLight::setShaderID(GLuint shaderProg, std::string name, bool displayon)
{
    _shaderID = shaderProg; _lightname = name;
    
    glm::vec4 amb = _lighingOn ? _ambient : glm::vec4(0.0f);
    glm::vec4 diff = _lighingOn ? _diffuse : glm::vec4(0.0f);
    glm::vec4 spec = _lighingOn ? _specular : glm::vec4(0.0f);

    GLint loc = glGetUniformLocation(_shaderID, (_lightname + ".position").c_str());
    glUniform3fv(loc, 1, glm::value_ptr(_position));

    loc = glGetUniformLocation(_shaderID, (_lightname + ".ambient").c_str());
    glUniform4fv(loc, 1, glm::value_ptr(amb));

    loc = glGetUniformLocation(_shaderID, (_lightname + ".diffuse").c_str());
    glUniform4fv(loc, 1, glm::value_ptr(diff));

    loc = glGetUniformLocation(_shaderID, (_lightname + ".specular").c_str());
    glUniform4fv(loc, 1, glm::value_ptr(spec));

    loc = glGetUniformLocation(_shaderID, (_lightname + ".constant").c_str());
    glUniform1f(loc, _constant);

    loc = glGetUniformLocation(_shaderID, (_lightname + ".linear").c_str());
    glUniform1f(loc, _linear);

    loc = glGetUniformLocation(_shaderID, (_lightname + ".quadratic").c_str());
    glUniform1f(loc, _quadratic);

    loc = glGetUniformLocation(_shaderID, "lightType");
    glUniform1i(loc, _type);

    if ( _type == LightType::SPOT ) {
        loc = glGetUniformLocation(_shaderID, (_lightname + ".direction").c_str());
        glUniform3fv(loc, 1, glm::value_ptr(_direction));

        loc = glGetUniformLocation(_shaderID, (_lightname + ".cutOff").c_str());
        glUniform1f(loc, _innerCutOff);

        loc = glGetUniformLocation(_shaderID, (_lightname + ".outerCutOff").c_str());
        glUniform1f(loc, _outerCutOff);
    }
    _displayOn = displayon;
    if ( _displayOn ) {
        _lightObj.setupVertexAttributes();
        _lightObj.setShaderID(_shaderID);
        _lightObj.setScale(glm::vec3(0.1f, 0.1f, 0.1f));
        _lightObj.setPos(_position);
    }
}

void CLight::updateToShader()
{
    //if (!_needsUpdate) return;

    GLint loc = glGetUniformLocation(_shaderID, (_lightname + ".position").c_str());
    glUniform3fv(loc, 1, glm::value_ptr(_position));

    loc = glGetUniformLocation(_shaderID, (_lightname + ".ambient").c_str());
    glUniform4fv(loc, 1, glm::value_ptr(_ambient));

    loc = glGetUniformLocation(_shaderID, (_lightname + ".diffuse").c_str());
    glUniform4fv(loc, 1, glm::value_ptr(_diffuse));

    loc = glGetUniformLocation(_shaderID, (_lightname + ".specular").c_str());
    glUniform4fv(loc, 1, glm::value_ptr(_specular));

    loc = glGetUniformLocation(_shaderID, (_lightname + ".constant").c_str());
    glUniform1f(loc, _constant);

    loc = glGetUniformLocation(_shaderID, (_lightname + ".linear").c_str());
    glUniform1f(loc, _linear);

    loc = glGetUniformLocation(_shaderID, (_lightname + ".quadratic").c_str());
    glUniform1f(loc, _quadratic);

    loc = glGetUniformLocation(_shaderID, "lightType");
    glUniform1i(loc, _type);

    if (_type == LightType::SPOT) {
        loc = glGetUniformLocation(_shaderID, (_lightname + ".direction").c_str());
        glUniform3fv(loc, 1, glm::value_ptr(_direction));

        loc = glGetUniformLocation(_shaderID, (_lightname + ".cutOff").c_str());
        glUniform1f(loc, _innerCutOff);

        loc = glGetUniformLocation(_shaderID, (_lightname + ".outerCutOff").c_str());
        glUniform1f(loc, _outerCutOff);

        loc = glGetUniformLocation(_shaderID, (_lightname + ".exponent").c_str());
        glUniform1f(loc, _exponent);
    }

    // _needsUpdate = false;
}

glm::vec3 CLight::getTarget() { return _target; }
glm::vec3 CLight::getDirection() { return _direction; }

void CLight::setTarget(const glm::vec3& target) {
    _target = target;
    _direction = glm::normalize(_target - _position);
    _type = LightType::SPOT;
}

void CLight::setCutOffDeg(float innerDeg, float outerDeg, float exponent) {
    _innerCutOff = glm::cos(glm::radians(innerDeg));
    _outerCutOff = glm::cos(glm::radians(outerDeg));
    _exponent = exponent;
    _type = LightType::SPOT;
}

void CLight::update(float dt)
{
    if (_motionOn) updateMotion(dt);
}
 
void CLight::updateMotion(float dt)
{
    glm::mat4 mxRot;
    glm::vec4 pos;
    _clock += dt;
    if (_clock >= 4.0f) _clock = 0.0f;
    
    // 計算當前角度和半徑
    float currentAngle = _clock * M_PI_2; // 每4秒轉90度 (π/2)
    float radius = glm::length(_posStart); // 計算起始位置到原點的距離
    
    std::cout << "=== LIGHT MOTION DEBUG ===" << std::endl;
    std::cout << "Light clock: " << _clock << std::endl;
    std::cout << "Light angle (radians): " << currentAngle << std::endl;
    std::cout << "Light angle (degrees): " << glm::degrees(currentAngle) << std::endl;
    std::cout << "Light start position: (" << _posStart.x << ", " << _posStart.y << ", " << _posStart.z << ")" << std::endl;
    std::cout << "Light radius from origin: " << radius << std::endl;
    
    mxRot = glm::rotate(glm::mat4(1.0f), currentAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    pos = glm::vec4(_posStart, 1.0f);
    _position = glm::vec3(mxRot * pos);
    
    std::cout << "Light current position: (" << _position.x << ", " << _position.y << ", " << _position.z << ")" << std::endl;
    std::cout << "=============================" << std::endl;
    
    setPos(_position);
    if (_displayOn) _lightObj.setPos(_position);
}


void CLight::draw()
{
    if( _displayOn ) _lightObj.draw();
}

void CLight::drawRaw()
{
    if (_displayOn) _lightObj.drawRaw();
}

CLight::LightType CLight::getType() const {
    return _type;
}

float CLight::getInnerCutOff() const {
    return _innerCutOff;
}

float CLight::getOuterCutOff() const {
    return _outerCutOff;
}

float CLight::getExponent() const {
    return _exponent;
}

bool CLight::isMotionOn() const {
    return _motionOn;
}

float CLight::getClock() const {
    return _clock;
}

glm::vec3 CLight::getStartPos() const {
    return _posStart;
}
