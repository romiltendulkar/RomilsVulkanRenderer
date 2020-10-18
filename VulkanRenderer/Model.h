#pragma once


#include "VulkanRaii.h"
#include <vulkan/vulkan.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>


class VulkanApplication;

struct VBufferSection
{
	vk::Buffer buffer = {};  // just a handle, no ownership for this buffer
	vk::DeviceSize offset = 0;
	vk::DeviceSize size = 0;

	VBufferSection() = default;

	VBufferSection(vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize size)
		: buffer(buffer)
		, offset(offset)
		, size(size)
	{}
};

struct VMeshPart
{
	VBufferSection vertex_buffer_section = {};
	VBufferSection index_buffer_section = {};
	VBufferSection material_uniform_buffer_section = {};
	size_t index_count = 0;
	vk::DescriptorSet material_descriptor_set = {};  // TODO: I still need a per-instance descriptor set


	// handles for images (no ownership or so)
	vk::ImageView albedo_map = {};
	vk::ImageView normal_map = {};



	VMeshPart(const VBufferSection& vertex_buffer_section, const VBufferSection& index_buffer_section, size_t index_count)
		: vertex_buffer_section(vertex_buffer_section)
		, index_buffer_section(index_buffer_section)
		, index_count(index_count)
	{}
};

template <class T>
void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 tex_coord;
	glm::vec3 normal;

	using index_t = uint32_t;

	bool operator==(const Vertex& other) const noexcept
	{
		return pos == other.pos && color == other.color && tex_coord == other.tex_coord && normal == other.normal;
	}

	size_t hash() const
	{
		size_t seed = 0;
		hash_combine(seed, pos);
		hash_combine(seed, color);
		hash_combine(seed, tex_coord);
		hash_combine(seed, normal);
		return seed;
	}
};

struct MaterialUbo
{
	int has_albedo_map;
	int has_normal_map;
};


struct MeshMaterialGroup // grouped by material
{
	std::vector<Vertex> vertices = {};
	std::vector<Vertex::index_t> vertex_indices = {};

	std::string albedo_map_path = "";
	std::string normal_map_path = "";
};

std::vector<char> readFile(const std::string& filename);

class VModel
{
public:
	VModel() = default;
	~VModel() = default;
	VModel(VModel&&) = default;
	VModel& operator= (VModel&&) = default;

	const std::vector<VMeshPart>& getMeshParts() const
	{
		return mesh_parts;
	}

	static VModel loadModelFromFile(const VulkanApplication& vulkanapp, const std::string& path
		, const vk::Sampler& texture_sampler, const vk::DescriptorPool& descriptor_pool,
		const vk::DescriptorSetLayout& material_descriptor_set_layout);

	VModel(const VModel&) = delete;
	VModel& operator= (const VModel&) = delete;

private:
	VulkanRaii<VkBuffer> buffer;
	VulkanRaii<VkDeviceMemory> buffer_memory;
	std::vector<VulkanRaii<VkImage>> images;
	std::vector<VulkanRaii<VkImageView>> imageviews;
	std::vector<VulkanRaii<VkDeviceMemory>> image_memories;
	VulkanRaii<VkBuffer> uniform_buffer;
	VulkanRaii<VkDeviceMemory> uniform_buffer_memory;

	std::vector<VMeshPart> mesh_parts;

};