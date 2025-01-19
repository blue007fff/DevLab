#include "Camera.h"
#include <algorithm>

namespace
{
	//https://www.khronos.org/opengl/wiki/Object_Mouse_Trackball
	double ProjectToSphere(double radius, double x, double y)
	{
		double z;
		double d = sqrt(x * x + y * y);
		if (d < radius * 0.70710678118654752440)
		{
			// Inside sphere if d< r/sqrt(2)
			z = sqrt(radius * radius - d * d);
		}
		else
		{
			// On hyperbola
			double t = (double)(radius / 1.41421356237309504880);
			z = t * t / d;
		}
		return z;
	}
}

glm::vec3 Camera::GetRight() const
{
	//glm::vec3 right(1.0f, 0.0f, 0.0f);
	//glm::quat quat = m_quat;
	//glm::quat vecQuat(0.0f, right); // vec3를 쿼터니언으로 변환
	//glm::quat result = quat * vecQuat * glm::conjugate(quat); // 회전 수행
	//return glm::vec3(result.x, result.y, result.z); // 결과를 vec3로 변환

	glm::mat4 tf = GetTransformation();
	return glm::vec3(tf[0]);
}

glm::vec3 Camera::GetUp() const
{
	//glm::vec3 up(0.0f, 1.0f, 0.0f);
	//glm::quat quat = m_quat;
	//glm::quat vecQuat(0.0f, up); // vec3를 쿼터니언으로 변환
	//glm::quat result = quat * vecQuat * glm::conjugate(quat); // 회전 수행
	//return glm::vec3(result.x, result.y, result.z); // 결과를 vec3로 변환

	glm::mat4 tf = GetTransformation();
	return glm::vec3(tf[1]);
}

glm::vec3 Camera::GetFoward() const
{
	glm::vec3 look(0.0f, 0.0f, 1.0f);
	glm::quat quat = m_quat;
	glm::quat vecQuat(0.0f, look); // vec3를 쿼터니언으로 변환
	glm::quat result = quat * vecQuat * glm::conjugate(quat); // 회전 수행
	return -glm::vec3(result.x, result.y, result.z); // 결과를 vec3로 변환

	/*glm::mat4 tf = GetTransformation();
	return -glm::vec3(tf[2]);*/
}

glm::vec3 Camera::GetLookAt() const
{
	return GetPos() + GetFoward() * m_lookDist;
}

void Camera::MoveFoward(float move)
{
	m_pos += GetFoward() * move;
}

void Camera::MoveRight(float move)
{
	m_pos += GetRight() * move;
}

void Camera::MoveUp(float move)
{
	m_pos += GetUp() * move;
}

void Camera::Turn(float amount)
{
	//m_quat = glm::rotate(m_quat, glm::radians(amount), glm::vec3(0, 1, 0));
	glm::quat q1 = glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(amount), glm::vec3(0, -1, 0));
	m_quat = m_quat * q1;
}

void Camera::Lookup(float amount)
{
	//m_quat = glm::rotate(m_quat, glm::radians(amount), glm::vec3(1, 0, 0));
	glm::quat q1 = glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(amount), glm::vec3(1, 0, 0));
	m_quat = m_quat * q1;
}
glm::mat4 Camera::GetTransformation() const
{
	return glm::translate(glm::mat4(1), m_pos) * glm::mat4_cast(m_quat);
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::inverse(GetTransformation());
}

void Camera::Trackball(int w, int h, glm::vec2 mousePrev, glm::vec2 mouseCurr)
{
	if (glm::length(mouseCurr - mousePrev) < 0.001)
		return;

	auto CalculateTrackballPoint = [w, h](glm::vec2 screenPoint) {
		float dTrackballSize = 0.5;// 1;
		float x = screenPoint.x / (float)w * 2.0f - 1.0f;
		//float y = -screenPoint.y / (float)h * 2.0f + 1.0f;
		float y = (h - screenPoint.y - 1) / (float)h * 2.0f - 1.0f;
		return glm::vec3(x, y, ProjectToSphere(dTrackballSize, x, y));
	};

	//Trackball 상의 좌표를 구한다.
	glm::vec3 p0 = CalculateTrackballPoint(mousePrev);
	glm::vec3 p1 = CalculateTrackballPoint(mouseCurr);

	//원점에서 각 점까지의 거리가 일정크기 이하라면 카메라는 제어되지 않는다.
	if (p0.length() < 0.000001f || p1.length() < 0.000001f)
		return;

	p0 = glm::normalize(p0);
	p1 = glm::normalize(p1);

	float angle = std::clamp(glm::dot(p0, p1), -1.0f, 1.0f);
	angle = acos(angle);

	//회전 축을 구한다.
	glm::vec3 rotAxis = glm::cross(p0, p1);
	rotAxis = glm::normalize(rotAxis);

	//회전 축을 [W]로 이동시킨다.
	//glm::mat4 cameraPose = GetTransformation();
	//rotAxis = cameraPose * glm::vec4(rotAxis, 0.0);
	rotAxis = m_quat * rotAxis;

	//회전 matrix를 구한다.
	glm::mat4 rot = glm::rotate(glm::mat4(1), -angle, rotAxis);

	//lootat 을 기준으로 회전.
	glm::vec3 lookat = GetLookAt();
	glm::mat4 trans = glm::translate(glm::mat4(1), -lookat);
	glm::mat4 invTrans = glm::translate(glm::mat4(1), lookat);
	glm::mat4 retTransformation = invTrans * rot * trans;

	/*{
		glm::quat rot = glm::rotate(glm::quat(1, 0, 0, 0), -angle, rotAxis);
		glm::vec3 newpos = rot * (-lookat);
		newpos = lookat + newpos;
		retTransformation = glm::mat4_cast(rot);
		retTransformation[3] = glm::vec4(newpos, 1);
	}*/

	//카메라를 회전 축에 맞춰 회전 시킨다.
	glm::mat4 tf = retTransformation * GetTransformation();
	m_pos = glm::vec3(tf[3]);
	m_quat = glm::quat_cast(tf);
	m_quat = glm::normalize(m_quat); // 오차 보정
}

void Camera::Zoom(float zoomRatio)
{
	//float zoomRatio = 0.01;
	glm::vec3 lookat = GetLookAt();

	float zoom = 1.0f - fabs(zoomRatio);
	if (zoomRatio > 0.0f) // zoom=out
		m_lookDist *= zoom;
	else // zoom-in
		m_lookDist *= 1.0f / zoom;

	m_pos = lookat - GetFoward() * GetLookDist();
}

void Camera::Lookat(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
	auto view = glm::lookAt(eye, center, up);
	auto tf = glm::inverse(view);
	m_pos = glm::vec3(tf[3]);
	m_quat = glm::quat_cast(tf);
	m_quat = glm::normalize(m_quat); // 오차 보정
}