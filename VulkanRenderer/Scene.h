#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

constexpr glm::vec3 vec_up = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 vec_right = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 vec_forward = glm::vec3(0.0f, 0.0f, -1.0f);

struct Camera
{
	glm::vec3 position = { 1.0f, 1.0f, 1.5f };
	glm::quat rotation = { 0.0f, 0.0f, 0.0f, 0.0f };
	float rotation_speed = glm::pi<float>();
	float move_speed = 2.0f;

	glm::mat4 getViewMatrix() const
	{
		return glm::transpose(glm::toMat4(rotation)) * glm::translate(glm::mat4(1.0f), -position); // equal to inv(TR) 
	}
};

class Scene
{
public:
	Scene();
	std::string model_file;
	float scale;
	glm::vec3 min_light_pos;
	glm::vec3 max_light_pos;
	float light_radius;
	int light_num;
	glm::vec3 camera_position;
	glm::quat camera_rotation;
};