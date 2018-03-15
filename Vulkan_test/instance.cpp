//
//  instance.cpp
//  Vulkan_test
//
//  Created by Danny on 15/03/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#include "instance.hpp"

Instance::Instance(const std::vector<const char*>& requiredValidationLayers,
				   const std::vector<const char*>& requiredExtensions) {
	
	VkInstanceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.ppEnabledLayerNames = requiredValidationLayers.data();
	info.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
	info.ppEnabledExtensionNames = requiredExtensions.data();
	info.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	
	VkResult result = vkCreateInstance(&info, NULL, &handle);
	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance");
	
	enumerateDevices();
}

std::vector<VkLayerProperties> Instance::getAvailableValidationLayers() {
	uint32_t instance_layer_count = 0;
	VkResult result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
	std::vector<VkLayerProperties> validationLayers(instance_layer_count);
	
	if (instance_layer_count > 0)
		result = vkEnumerateInstanceLayerProperties(&instance_layer_count, validationLayers.data());
	
	return validationLayers;
}

std::vector<VkExtensionProperties> Instance::getAvailableExtensions() {
	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
	return extensions;
}

void Instance::enumerateDevices() {
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(handle, &device_count, nullptr);
	if(device_count) {
		std::vector<VkPhysicalDevice> physical_devices(device_count);
		vkEnumeratePhysicalDevices(handle, &device_count, physical_devices.data());
		
		for(auto& vDevice: physical_devices)
			physicalDevices.emplace_back(*this, vDevice);
		
	}
}
