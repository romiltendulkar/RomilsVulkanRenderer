#pragma once


#pragma once

#include <vulkan/vulkan.h>
#include <functional>


template <typename T>
class VulkanRaii
{
public:
	using obj_t = T;

	VulkanRaii()
		: object(nullptr)
		, deleter([](T&) {})
	{}

	VulkanRaii(T obj, std::function<void(T&)> deleter)
		: object(obj)
		, deleter(deleter)
	{}

	~VulkanRaii()
	{
		cleanup();
	}

	VulkanRaii(VulkanRaii<T>&& other)
		: object(nullptr) // to be swapped to "other"
		, deleter([](T&) {}) // deleter will be copied in case there is still use for the old container
	{
		swap(*this, other);
	}
	VulkanRaii<T>& operator=(VulkanRaii<T>&& other)
	{
		swap(*this, other);
		return *this;
	}
	friend void swap(VulkanRaii<T>& first, VulkanRaii<T>& second)
	{
		using std::swap;
		swap(first.object, second.object);
		swap(first.deleter, second.deleter);
	}

	T& get()
	{
		return object;
	}

	const T& get() const
	{
		return object;
	}

	T* data()
	{
		return &object;
	}

	VulkanRaii<T>& operator=(const VulkanRaii<T>&) = delete;
	VulkanRaii(const VulkanRaii<T>&) = delete;

private:
	T object;
	std::function<void(T&)> deleter;

	void cleanup()
	{
		deleter(object);
	}
};