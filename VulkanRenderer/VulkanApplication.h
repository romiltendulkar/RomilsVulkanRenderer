#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include "InputManager.h"
#include "Scene.h"
#include "VulkanRaii.h"
#include "Utilities.h"
#include "Model.h"

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

const std::vector<const char*> VALIDATION_LAYERS = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> DEVICE_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const uint32_t WINDOW_WIDTH = 1920;
const uint32_t WINDOW_HEIGHT = 1080;

const int MAX_POINT_LIGHT_COUNT = 10000; 
const int MAX_POINT_LIGHT_PER_TILE = 1023;
const int TILE_SIZE = 16;

struct PointLight
{
public:
	//glm::vec3 pos = { 0.0f, 1.0f, 0.0f };
	glm::vec3 pos;
	float radius = { 5.0f };
	glm::vec3 intensity = { 1.0f, 1.0f, 1.0f };
	float padding;

	PointLight() {}
	PointLight(glm::vec3 pos, float radius, glm::vec3 intensity)
		: pos(pos), radius(radius), intensity(intensity)
	{};
};



// uniform buffer object for model transformation
struct SceneObjectUbo
{
	glm::mat4 model;
};

// uniform buffer object for camera
struct CameraUbo
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 projview;
	glm::vec3 cam_pos;
};

struct PushConstantObject
{
	glm::ivec2 viewport_size;
	glm::ivec2 tile_nums;
	int debugview_index; // TODO: separate this and only have it in debug mode?

	PushConstantObject(int viewport_size_x, int viewport_size_y, int tile_num_x, int tile_num_y, int debugview_index = 0)
		: viewport_size(viewport_size_x, viewport_size_y),
		tile_nums(tile_num_x, tile_num_y),
		debugview_index(debugview_index)
	{}
};

struct QueueFamilyIndices
{
	int graphics_family = -1;
	int present_family = -1;

	bool isComplete()
	{
		return graphics_family >= 0 && present_family >= 0; //&& compute_family >= 0;
	}

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;

		uint32_t queuefamily_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamily_count, nullptr);

		std::vector<VkQueueFamilyProperties> queuefamilies(queuefamily_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamily_count, queuefamilies.data());

		int i = 0;
		for (const auto& queuefamily : queuefamilies)
		{
			if (queuefamily.queueCount > 0 && (queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				auto support_compute = static_cast<bool>(queuefamily.queueFlags & VK_QUEUE_COMPUTE_BIT);
				auto enough_size = (queuefamily.queueCount >= 2);

				if (!support_compute)
				{
					std::cout << "Found a graphics queue family, but it doesn't support compute." << std::endl;
				}
				if (!enough_size) // using a second queue to do light culling
				{
					std::cout << "Found a graphics queue family, but it doesn't enough queue count" << std::endl;
				}

				if (support_compute && enough_size)
				{
					indices.graphics_family = i;
				}
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (queuefamily.queueCount > 0 && presentSupport)
			{
				indices.present_family = i;
			}
			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;

	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// Getting supported surface formats
		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
		if (format_count != 0)
		{
			details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
		}

		// Getting supported present modes
		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
		if (present_mode_count != 0)
		{
			details.present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
		}

		return details;
	}
};

class VulkanApplication
{
public:
	void Run();
	auto getQueueFamilyIndices() const
	{
		return queue_family_indices;
	}

	vk::PhysicalDevice getPhysicalDevice() const
	{
		return physical_device;
	}

	const vk::PhysicalDeviceProperties& getPhysicalDeviceProperties() const
	{
		return physical_device_properties;
	}

	vk::Device getDevice() const
	{
		return graphics_device.get();
	}

	vk::Queue getGraphicsQueue() const
	{
		return graphics_queue;
	}

	vk::Queue getPresentQueue() const
	{
		return present_queue;
	}

	vk::Queue getComputeQueue() const
	{
		return compute_queue;
	}

	vk::SurfaceKHR getWindowSurface() const
	{
		return window_surface.get();
	}

	vk::CommandPool getGraphicsCommandPool() const
	{
		return graphics_queue_command_pool.get();
	}

	vk::CommandPool getComputeCommandPool() const
	{
		return compute_queue_command_pool.get();
	}
	std::pair<int, int> getWindowFrameBufferSize() const
	{
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(mpWindow, &framebuffer_width, &framebuffer_height);
		return std::make_pair(framebuffer_width, framebuffer_height);
	}
	int getDebugViewIndex() const
	{
		return debug_view_index;
	}



private:
	////////////////////////////////////////////////////
	// Window Functions
	////////////////////////////////////////////////////
	void InitWindow();
	void Loop();
	void Cleanup();
	void FrameBufferCallback(GLFWwindow* window, int width, int height);

	////////////////////////////////////////////////////
	// Vulkan Initializing Functions
	////////////////////////////////////////////////////
	void InitVulkan();
	void CreateInstance();
	void setupDebugCallback();
	void createWindowSurface();
	void pickPhysicalDevice();
	void findQueueFamilyIndices();
	void createLogicalDevice();
	void createCommandPools();
	static void DestroyDebugReportCallbackEXT(VkInstance instance
		, VkDebugReportCallbackEXT callback
		, const VkAllocationCallbacks* pAllocator);

	static VkResult CreateDebugReportCallbackEXT(VkInstance instance
		, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator
		, VkDebugReportCallbackEXT* pCallback);


	void resize(int width, int height);
	void changeDebugViewIndex(int target_view)
	{
		debug_view_index = target_view % 5;
		recreateSwapChain(); // TODO: change this to a state modification and handle the recreation before update
	}
	void requestDraw(float deltatime);
	void cleanUp();

	void setCamera(const glm::mat4& view, const glm::vec3 campos);


	void initialize()
	{
		createSwapChain();
		createSwapChainImageViews();
		createRenderPasses();
		createDescriptorSetLayouts();
		createGraphicsPipelines();
		createComputePipeline();
		createDepthResources();
		createFrameBuffers();
		createTextureSampler();
		createUniformBuffers();
		createLights();
		createDescriptorPool();
		model = VModel::loadModelFromFile(*this, mScene->model_file, texture_sampler.get(), descriptor_pool.get(), material_descriptor_set_layout.get());
		createSceneObjectDescriptorSet();
		createCameraDescriptorSet();
		createIntermediateDescriptorSet();
		updateIntermediateDescriptorSet();
		createLigutCullingDescriptorSet();
		createLightVisibilityBuffer(); // create a light visiblity buffer and update descriptor sets, need to rerun after changing size
		createGraphicsCommandBuffers();
		createLightCullingCommandBuffer();
		createDepthPrePassCommandBuffer();
		createSemaphores();
	}

	void recreateSwapChain()
	{
		vkDeviceWaitIdle(graphics_device.get());

		createSwapChain();
		createSwapChainImageViews();
		createRenderPasses();
		createGraphicsPipelines();
		createDepthResources();
		createFrameBuffers();
		createLightVisibilityBuffer(); // since it's size will scale with window;
		updateIntermediateDescriptorSet();
		createGraphicsCommandBuffers();
		createLightCullingCommandBuffer(); // it needs light_visibility_buffer_size, which is changed on resize
		createDepthPrePassCommandBuffer();
	}

	void createSwapChain();
	void createSwapChainImageViews();
	void createRenderPasses();
	void createDescriptorSetLayouts();
	void createGraphicsPipelines();
	void createDepthResources();
	void createFrameBuffers();
	void createTextureSampler();
	void createUniformBuffers();
	void createLights();
	void createDescriptorPool();
	void createSceneObjectDescriptorSet();
	void createCameraDescriptorSet();
	void createIntermediateDescriptorSet();
	void updateIntermediateDescriptorSet();
	void createGraphicsCommandBuffers();
	void createSemaphores();

	void createComputePipeline();
	void createLigutCullingDescriptorSet();
	void createLightVisibilityBuffer();
	void createLightCullingCommandBuffer();

	void createDepthPrePassCommandBuffer();

	void updateUniformBuffers(float deltatime);
	void drawFrame();

	VulkanRaii<VkShaderModule> createShaderModule(const std::vector<char>& code);


	void CheckInput(float deltatime);

	////////////////////////////////////////////////////
	// Vulkan Query Functions
	////////////////////////////////////////////////////
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR window_surface);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	
	////////////////////////////////////////////////////
	// Variables
	////////////////////////////////////////////////////	
public:
	GLFWwindow* mpWindow;
	InputManager* mpInputManager;
	Camera mCamera;
	uint32_t mWidth = 1920;
	uint32_t mHeight = 1080;
	Scene* mScene = new Scene();
private:
	VulkanRaii<vk::Instance> instance;
	VUtility *utility;
	VulkanRaii<vk::DebugReportCallbackEXT> callback;
	VulkanRaii<vk::Device> graphics_device;
	VulkanRaii<vk::SurfaceKHR> window_surface;

	QueueFamilyIndices queue_family_indices;
	VkPhysicalDevice physical_device;
	VkDevice graphicsdevice;
	vk::Device device;
	vk::Queue graphics_queue;
	vk::Queue present_queue;
	vk::Queue compute_queue;
	vk::CommandPool graphics_command_pool;
	vk::CommandPool compute_command_pool;

	VulkanRaii<vk::SwapchainKHR> swap_chain;
	std::vector<VkImage> swap_chain_images;
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;
	std::vector<VulkanRaii<vk::ImageView>> swap_chain_imageviews;
	std::vector<VulkanRaii<vk::Framebuffer>> swap_chain_framebuffers;
	VulkanRaii<vk::Framebuffer> depth_pre_pass_framebuffer;

	VulkanRaii<vk::RenderPass> render_pass;
	VulkanRaii<vk::RenderPass> depth_pre_pass; // the depth prepass which happens before formal render pass

	VulkanRaii<vk::DescriptorSetLayout> object_descriptor_set_layout;
	VulkanRaii<vk::DescriptorSetLayout> camera_descriptor_set_layout;
	VulkanRaii<vk::DescriptorSetLayout> material_descriptor_set_layout;
	VulkanRaii<VkPipelineLayout> pipeline_layout;
	VulkanRaii<VkPipeline> graphics_pipeline;
	VulkanRaii<vk::PipelineLayout> depth_pipeline_layout;
	VulkanRaii<vk::Pipeline> depth_pipeline;

	VulkanRaii<vk::DescriptorSetLayout> light_culling_descriptor_set_layout;  // shared between compute queue and graphics queue
	VulkanRaii<vk::DescriptorSetLayout> intermediate_descriptor_set_layout; // which is exclusive to compute queue
	VulkanRaii<VkPipelineLayout> compute_pipeline_layout;
	VulkanRaii<VkPipeline> compute_pipeline;
	vk::CommandBuffer light_culling_command_buffer = {};

	std::vector<VkCommandBuffer> command_buffers; // buffers will be released when pool destroyed
	vk::CommandBuffer depth_prepass_command_buffer;

	VulkanRaii<vk::Semaphore> image_available_semaphore;
	VulkanRaii<vk::Semaphore> render_finished_semaphore;
	VulkanRaii<vk::Semaphore> lightculling_completed_semaphore;
	VulkanRaii<vk::Semaphore> depth_prepass_finished_semaphore;

	// for depth
	VulkanRaii<VkImage> depth_image;
	VulkanRaii<VkDeviceMemory> depth_image_memory;
	VulkanRaii<VkImageView> depth_image_view;

	// texture image
	VulkanRaii<VkImage> texture_image;
	VulkanRaii<VkDeviceMemory> texture_image_memory;
	VulkanRaii<VkImageView> texture_image_view;
	VulkanRaii<VkImage> normalmap_image;
	VulkanRaii<VkDeviceMemory> normalmap_image_memory;
	VulkanRaii<VkImageView> normalmap_image_view;
	VulkanRaii<VkSampler> texture_sampler;

	// uniform buffers
	VulkanRaii<VkBuffer> object_staging_buffer;
	VulkanRaii<VkDeviceMemory> object_staging_buffer_memory;
	VulkanRaii<VkBuffer> object_uniform_buffer;
	VulkanRaii<VkDeviceMemory> object_uniform_buffer_memory;
	VulkanRaii<VkBuffer> camera_staging_buffer;
	VulkanRaii<VkDeviceMemory> camera_staging_buffer_memory;
	VulkanRaii<VkBuffer> camera_uniform_buffer;
	VulkanRaii<VkDeviceMemory> camera_uniform_buffer_memory;

	VulkanRaii<VkDescriptorPool> descriptor_pool;
	VkDescriptorSet object_descriptor_set;
	vk::DescriptorSet camera_descriptor_set;
	VkDescriptorSet light_culling_descriptor_set;
	vk::DescriptorSet intermediate_descriptor_set;

	VModel model;


	VulkanRaii<VkBuffer> pointlight_buffer;
	VulkanRaii<VkDeviceMemory> pointlight_buffer_memory;
	VulkanRaii<VkBuffer> lights_staging_buffer;
	VulkanRaii<VkDeviceMemory> lights_staging_buffer_memory;
	VkDeviceSize pointlight_buffer_size;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> vertex_indices;

	std::vector<PointLight> pointlights;

	// This storage buffer stores visible lights for each tile
	// which is output from the light culling compute shader
	// max MAX_POINT_LIGHT_PER_TILE point lights per tile
	VulkanRaii<VkBuffer> light_visibility_buffer;
	VulkanRaii<VkDeviceMemory> light_visibility_buffer_memory;
	VkDeviceSize light_visibility_buffer_size = 0;

	int window_framebuffer_width;
	int window_framebuffer_height;

	glm::mat4 view_matrix;
	glm::vec3 cam_pos;
	int tile_count_per_row;
	int tile_count_per_col;
	int debug_view_index = 0;

	VulkanRaii<vk::CommandPool> graphics_queue_command_pool;
	VulkanRaii<vk::CommandPool> compute_queue_command_pool;
	vk::PhysicalDeviceProperties physical_device_properties;

};

