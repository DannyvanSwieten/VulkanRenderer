//
//  vulkan_renderer.hpp
//  Vulkan_test
//
//  Created by Danny on 15/03/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#pragma once

#include <vulkan/vulkan.hpp>

class VulkanRenderer {
public:
	VulkanRenderer(void* nativeWindowHandle = nullptr);

private:
	
	void chooseBestDevice(const std::vector<vk::PhysicalDevice>&);
	void createPlatformSpecificSurface();
	bool checkSwapChainCompatibilityForDevice(const vk::PhysicalDevice& device);
	void createLogicalDeviceAndPresentQueue();
	void chooseSurfaceFormatForSwapChain();
	void choosePresentModeForSwapChain();
	
private:
	
	vk::Instance instance;
	vk::PhysicalDevice physicalDevice;
	vk::Device logicalDevice;
	vk::Queue presentQueue;
	vk::GraphicsPipelineCreateInfo pipelineState;
	vk::Pipeline pipeline;
	vk::SurfaceKHR surface;
	vk::SurfaceCapabilitiesKHR surfaceCababilities;
	std::vector<vk::SurfaceFormatKHR> supportedSurfaceFormats;
	std::vector<vk::PresentModeKHR> supportedPresentModes;
	
	vk::SurfaceFormatKHR swapChainFormat;
	vk::PresentModeKHR swapChainPresentMode;
	
	void* nativeWindowHandle = nullptr;
	
	uint32_t graphicsQueueIndex = 0;
	uint32_t presentQueueIndex = 0;
};
