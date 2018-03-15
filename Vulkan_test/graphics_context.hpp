//
//  graphics_context.hpp
//  Vulkan_test
//
//  Created by Danny on 27/02/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#pragma once

#ifdef __APPLE__
#define VK_USE_PLATFORM_MACOS_MVK
#endif

#include <vulkan/vulkan.hpp>

struct PhysicalDevice {
	
	Instance& instance;
	VkPhysicalDevice device;
	std::vector<VkExtensionProperties> extensions;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	
	PhysicalDevice(Instance& instance, VkPhysicalDevice d): device(d) {
		
		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);
		
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
		queueFamilyProperties.resize(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queueFamilyProperties.data());
		
		uint32_t deviceExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, nullptr);
		extensions.resize(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, extensions.data());
	}
	
	bool supportsComputeQueue() const {
		for(const auto& family: queueFamilyProperties) {
			if(family.queueCount && family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
				return true;
			}
		}
		
		return false;
	}
	
	bool supportsGraphicsQueue() const {
		for(const auto& family: queueFamilyProperties) {
			if(family.queueCount && family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				return true;
			}
		}
		
		return false;
	}
	
	bool supportsTransferQueue() const {
		for(const auto& family: queueFamilyProperties) {
			if(family.queueCount && family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
				return true;
			}
		}
		
		return false;
	}
};

class Device {
public:
	Device(const PhysicalDevice& device, const Instance& instance, const void* nativeWindowHandle = nullptr);
	~Device();
	
	const PhysicalDevice& physicalDevice;
	const Instance& instance;
	
private:
	
	void createSurfaceFromNativeHandle(const void* nativeHandle);
	
private:
	
	VkDevice device				= VK_NULL_HANDLE;
	VkQueue graphicsQueue 		= VK_NULL_HANDLE;
	VkSurfaceKHR displaySurface	= VK_NULL_HANDLE;
};
