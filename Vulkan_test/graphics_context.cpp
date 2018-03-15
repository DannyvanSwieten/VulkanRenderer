//
//  graphics_context.cpp
//  Vulkan_test
//
//  Created by Danny on 27/02/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#include "graphics_context.hpp"

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
