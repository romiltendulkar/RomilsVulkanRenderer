#include "Scene.h"

Scene::Scene()
{
	model_file = "Models/sponza.obj";
	scale = 0.01f;
	min_light_pos = glm::vec3{ -20, 0, -20 };
	max_light_pos = glm::vec3{ 20, 20, 20 };
	light_radius = 3.0f;
	light_num = 1000;
	camera_position = glm::vec3{ 12.7101822f, 1.87933588f, -0.0333303586f };
	camera_rotation = glm::quat{ 0.717312694f, -0.00208670134f, 0.696745396f, 0.00202676491f };
}