#include "CCamera.h"
#include <glm/gtc/matrix_transform.hpp>  // For glm::lookAt and glm::perspective
#include <iostream>
#include "typedefs.h"
#include "CollisionManager.h"
extern CollisionManager g_collisionManager;

using namespace glm;

// Singleton instance
CCamera& CCamera::getInstance()
{
	static CCamera instance;
	return instance;
}

CCamera::CCamera()
{
	_theta = 0; _phi = 0;	 // 角度由  updateViewCenter 來計算 
	_radius = 0.0f; // 半徑由  updateViewCenter 來計算

	// Default projection type
	_type = Type::PERSPECTIVE;
	_view = glm::vec3(5.0f, 5.0f, 5.0f); // 鏡頭位置
	_center = glm::vec3(0.0f, 0.0f, 0.0f); // 目標點
	_up = glm::vec3(0.0f, 1.0f, 0.0f);
	// 預設鏡頭從 (5,5,5) 看向 (0,0,0) ，利用 updateViewMatrix 來計算
	updateViewCenter(_view, _center);
	_mxProj = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 1000.0f);
	_mxViewProj = _mxProj * _mxView;

	_bviewUpdate = true;
	_bprojUpdate = true;
}

void CCamera::updatePerspective(float fovy, float aspect, float zNear, float zFar)
{
	_mxProj = glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
	_type = Type::PERSPECTIVE;
	_bprojUpdate = true;
}

void CCamera::updateOrthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
	_mxProj = glm::ortho(left, right, bottom, top, zNear, zFar);
	_type = Type::ORTHOGRAPHIC;
	_bprojUpdate = true;
}

void CCamera::processMouseMovement(float deltaX, float deltaY, float sensitivity)
{
	_theta += deltaX * sensitivity;
	_phi -= deltaY * sensitivity;

	// 限制 _phi 值避免接近 0 或 pi
	if (_phi < 0.1f) _phi = 0.1f;
	if (_phi > M_PI- 0.1f) _phi = M_PI - 0.1f;

	updateViewMatrix(_theta, _phi);
}

void CCamera::processMouseScroll(float deltaScroll, float zoomSensitivity)
{
	_radius -= deltaScroll * zoomSensitivity;
	if (_radius < 1.0f) _radius = 1.0f;  // 限制最小距離
	updateViewMatrix();
}

const glm::vec3& CCamera::getViewLocation()
{
	return _view;
}

void CCamera::updateView(const glm::vec3& view)
{
	_view = view;
	glm::vec3 offset = _view - _center;
	_radius = glm::length(offset);

	if (_radius < 1e-5f) {
		_radius = 1.0f;
		_theta = 0.0f;
		_phi = M_PI / 2.0f;  // 水平
	}
	else {
		_theta = -atan2(-offset.z, offset.x);
		_phi = acos(glm::clamp(offset.y / _radius, -1.0f, 1.0f));
	}
	updateViewMatrix();
}

void CCamera::updateCenter(const glm::vec3& center)
{
	_center = center;
	glm::vec3 offset = _view - _center;
	_radius = glm::length(offset);

	// 避免除以 0
	if (_radius < 1e-5f) {
		_radius = 1.0f;
		_theta = 0.0f;
		_phi = M_PI / 2.0f;  // 水平朝向
	}
	else {
		// Y 軸朝上，球座標計算
		_theta = -atan2(-offset.z, offset.x);
		_phi = acos(glm::clamp(offset.y / _radius, -1.0f, 1.0f));
	}
	updateViewMatrix();
}

void CCamera::updateViewCenter(const glm::vec3& view, const glm::vec3& center)
{
	_view = view; _center = center;

	glm::vec3 offset = _view - _center;
	_radius = glm::length(offset);

	// 避免除以 0
	if (_radius < 1e-5f) {
		_radius = 1.0f;
		_theta = 0.0f;
		_phi = M_PI / 2.0f;  // 水平朝向
	}
	else {
		// Y 軸朝上，球座標計算
		_theta = -atan2(-offset.z, offset.x);
		_phi = acos(glm::clamp(offset.y / _radius, -1.0f, 1.0f));
	}
	updateViewMatrix();
}

void CCamera::updateViewMatrix()
{
	_mxView = glm::lookAt(_view, _center, _up);
	_bviewUpdate = true;
}

// Convert spherical coordinates to Cartesian coordinates
void CCamera::updateViewMatrix(float theta, float phi)
{
	_view.x = _center.x + _radius * sin(phi) * cos(theta);
	_view.y = _center.y + _radius * cos(phi);
	_view.z = _center.z + _radius * sin(phi) * sin(theta);
	_mxView = glm::lookAt(_view, _center, _up);
	_bviewUpdate = true;
}

void CCamera::updateRadius(float delta) {
//    float oldRadius = _radius; // Store the old radius
    _radius += delta;

    // Apply the minimum radius constraint
    if (_radius < 1.0f) {
        _radius = 1.0f;
    }

    // Calculate the potential new camera position
    glm::vec3 potentialView;
    potentialView.x = _center.x + _radius * sin(_phi) * cos(_theta);
    potentialView.y = _center.y + _radius * cos(_phi);
    potentialView.z = _center.z + _radius * sin(_phi) * sin(_theta);

    // Get the current camera location (before radius update) for collision check
    glm::vec3 currentViewLocation = _view; // This is the camera's current position

    // Use CollisionManager to get a safe movement from current to potential position
    // We can think of this as moving from 'currentViewLocation' to 'potentialView'
    // and letting the collision manager tell us the safe displacement.
    glm::vec3 desiredMovement = potentialView - currentViewLocation;
    glm::vec3 safeMovement = g_collisionManager.getSafeMovement(desiredMovement, currentViewLocation);

    // Apply the safe movement to the current view to get the final new view position
    glm::vec3 newView = currentViewLocation + safeMovement;

    // Update the camera's position
    _view = newView;

    // Now, re-calculate the radius based on the *actual* new view position
    // This is important because if safeMovement was less than desiredMovement,
    // the actual radius might be different from the initially calculated _radius.
    _radius = glm::length(_view - _center);

    // Ensure _radius is at least 1.0f after recalculation to avoid issues with center.
    if (_radius < 1.0f) {
        _radius = 1.0f;
        // If it snaps to 1.0f, recalculate _view to be exactly at 1.0f radius from _center
        _view.x = _center.x + _radius * sin(_phi) * cos(_theta);
        _view.y = _center.y + _radius * cos(_phi);
        _view.z = _center.z + _radius * sin(_phi) * sin(_theta);
    }
    
    // Update the view matrix with the (potentially corrected) _view position
    updateViewMatrix();
}

const glm::mat4& CCamera::getProjectionMatrix() 
{
	return _mxProj;
}

const glm::mat4& CCamera::getViewMatrix()
{
	return _mxView;
}

const glm::mat4& CCamera::getViewProjectionMatrix() const
{
	if ( _bviewUpdate || _bprojUpdate ) {
		_mxViewProj = _mxProj * _mxView;
		_bviewUpdate = false;
		_bprojUpdate = false;
	}
	return _mxViewProj;
}

CCamera::Type CCamera::getProjectionType() const
{
	return _type;
}
