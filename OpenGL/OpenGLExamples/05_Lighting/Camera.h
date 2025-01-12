#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>  // value_ptr 포함
#include <glm/gtc/quaternion.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
	void SetPos(glm::vec3 pos) { m_pos = pos; }
	void SetQuat(glm::quat quat) { m_quat = quat; }
	glm::vec3 GetPos() const { return m_pos; }
	glm::vec3 GetRight() const;
	glm::vec3 GetUp() const;
	glm::vec3 GetFoward() const;
	glm::vec3 GetLookAt() const;
	float GetLookDist() const { return m_lookDist; }

	void MoveFoward(float move);
	void MoveRight(float move);
	void MoveUp(float move);
	void Turn(float amount);

	void Trackball(int w, int h, glm::vec2 mousePrev, glm::vec2 mouseCurr);
	void Zoom(float zoomRatio);

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetTransformation() const;


private:
	glm::vec3 m_pos{ 0.0f, 0.0f, 0.0f };
	glm::quat m_quat{1.0f, 0.0f, 0.0f, 0.0f};
	float m_lookDist{ 5.0f };
};