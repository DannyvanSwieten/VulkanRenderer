//
//  instance.hpp
//  Vulkan_test
//
//  Created by Danny on 15/03/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#pragma once

#include <vulkan/vulkan.hpp>

class Instance {
public:
	Instance(const std::vector<const char*>& requiredValidationLayers,
			 const std::vector<const char*>& requiredExtensions);
	
	static std::vector<VkLayerProperties> getAvailableValidationLayers();
	static std::vector<VkExtensionProperties> getAvailableExtensions();
	
	const VkInstance getHandle() const {
		return handle;
	}
	
	const std::vector<vk::PhysicalDevice>& getPhysicalDevices() const {
		return physicalDevices;
	}
	
private:
	
	void enumerateDevices();
	
private:
	
	vk::Instance handle;
	std::vector<vk::PhysicalDevice> physicalDevices;
};
