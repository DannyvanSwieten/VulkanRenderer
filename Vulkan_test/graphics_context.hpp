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
	
	VkPhysicalDevice device;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	
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
	
	bool supportsCopyQueue() const {
		for(const auto& family: queueFamilyProperties) {
			if(family.queueCount && family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
				return true;
			}
		}
		
		return false;
	}
};

class Instance {
public:
	Instance(const std::vector<const char*>& requiredValidationLayers,
			 const std::vector<const char*>& requiredExtensions);
	
	static std::vector<VkLayerProperties> getAvailableValidationLayers();
	static std::vector<VkExtensionProperties> getAvailableExtensions();
	
	const VkInstance getHandle() const {
		return handle;
	}
	
	const std::vector<PhysicalDevice>& getPhysicalDevices() const {
		return physicalDevices;
	}
	
private:
	
	void enumerateDevices();
	
private:
	
	VkInstance handle = VK_NULL_HANDLE;
	std::vector<PhysicalDevice> physicalDevices;
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
