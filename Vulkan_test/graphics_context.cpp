//
//  graphics_context.cpp
//  Vulkan_test
//
//  Created by Danny on 27/02/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#include "graphics_context.hpp"

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
		physicalDevices.resize(device_count);
		std::vector<VkPhysicalDevice> physical_devices(device_count);
		vkEnumeratePhysicalDevices(handle, &device_count, physical_devices.data());
		
		for(auto i = 0; i < device_count; i++) {
			const auto device = physical_devices[i];
			physicalDevices[i].device = device;
			vkGetPhysicalDeviceProperties(device, &physicalDevices[i].properties);
			vkGetPhysicalDeviceFeatures(device, &physicalDevices[i].features);
			
			uint32_t queue_family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
			physicalDevices[i].queueFamilyProperties.resize(queue_family_count);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, physicalDevices[i].queueFamilyProperties.data());
		}
	}
}

Device::Device(const PhysicalDevice& d, const Instance& instance, const void* nativeWindowHandle):
physicalDevice(d), instance(instance) {
	
	if(nativeWindowHandle)
		createSurfaceFromNativeHandle(nativeWindowHandle);

	float priorities[] = { 1.0 };
	const uint32_t familyCount = static_cast<uint32_t>(d.queueFamilyProperties.size());
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for(auto i = 0; i < familyCount; i++) {
		
		VkBool32 supported = false;
		if(nativeWindowHandle) {
			vkGetPhysicalDeviceSurfaceSupportKHR(d.device, i, displaySurface, &supported);
			if(!supported)
				continue;
		}
		
		VkDeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = i;
		queueCreateInfo.flags = 0;
		queueCreateInfo.pQueuePriorities = priorities;
		queueCreateInfos.emplace_back(queueCreateInfo);
	}
	
	if(!queueCreateInfos.size())
		exit(-1);
	
	VkDeviceCreateInfo deviceCreateInfo{};
	memset(&deviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.enabledLayerCount = 0; // This is deprecated and ignored.
	deviceCreateInfo.ppEnabledLayerNames = nullptr; // This is deprecated and ignored.
	deviceCreateInfo.ppEnabledExtensionNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pEnabledFeatures = &d.features;
	
	VkResult result = vkCreateDevice(d.device, &deviceCreateInfo, nullptr, &device);
	if(result != VK_SUCCESS)
		exit(-1);
}

Device::~Device() {
	if(device)
		vkDestroyDevice(device, nullptr);
}

void Device::createSurfaceFromNativeHandle(const void *nativeHandle) {
#ifdef VK_USE_PLATFORM_MACOS_MVK
	VkMacOSSurfaceCreateInfoMVK info;
	
	info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	info.pNext = nullptr;
	info.flags = 0;
	info.pView = nativeHandle;
	
	VkResult result = vkCreateMacOSSurfaceMVK(instance.getHandle(), &info, nullptr, &displaySurface);
	if(result != VK_SUCCESS)
		exit(-1);
#endif
}
