//
//  vulkan_renderer.hpp
//  Vulkan_test
//
//  Created by Danny on 15/03/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include "resource_descriptors.hpp"

class VulkanRenderer {
public:
	VulkanRenderer(const DeviceRequirements& reqs);

private:
	
	void chooseBestDevice(const std::vector<vk::PhysicalDevice>&, const DeviceRequirements& reqs);
	void createPlatformSpecificSurface(void* nativeWindowHandle);
	bool checkSwapChainCompatibilityForDevice(const vk::PhysicalDevice& device, const DeviceRequirements& reqs);
	void createLogicalDeviceAndPresentQueue(const DeviceRequirements& reqs);
	void chooseSurfaceFormatForSwapChain();
	void choosePresentModeForSwapChain();
	void createSwapChain();
	void createCommandPool();
	void createDescriptorPool();
	
	vk::VertexInputAttributeDescription createAttributeDescription(const VertexAttributeDescriptor&);
	
public:
	
	resource_handle_t createShaderModule(const std::string& source);
	resource_handle_t createShaderModuleFromSpirV(const std::vector<uint32_t> instructions);
	
	resource_handle_t createRenderpass(const RenderPassDescriptor&);
	resource_handle_t createRenderPipeline(const RenderPipelineDescriptor& );
	
private:
	
	// An instance and entrypoint to the API
	vk::Instance instance;
	
	// The physical (hardware) device we connect to.
	vk::PhysicalDevice physicalDevice;
	// The software wrapper around the physical device.
	vk::Device logicalDevice;
	
	// The queue we use to present images to the screen.
	vk::Queue presentQueue;
	
	// The information used to create a pipeline on the gpu that's ready to render.
	vk::GraphicsPipelineCreateInfo pipelineState;
	// The pipeline itself
	vk::Pipeline pipeline;
	
	// If the context is given a native window handle than these are
	vk::SurfaceKHR surface;
	vk::SurfaceCapabilitiesKHR surfaceCababilities;
	std::vector<vk::SurfaceFormatKHR> supportedSurfaceFormats;
	std::vector<vk::PresentModeKHR> supportedPresentModes;
	
	// The swapchain and it's images and imageviews
	vk::SwapchainKHR swapChain;
	vk::SurfaceFormatKHR swapChainFormat;
	vk::PresentModeKHR swapChainPresentMode;
	std::vector<vk::Image> swapChainImages;
	std::vector<vk::ImageView> swapChainImageViews;
	std::vector<vk::Framebuffer> swapChainFrameBuffers;
	vk::Image depthBuffer;
	vk::ImageView depthBufferView;
	
	vk::RenderPass renderPass;
	
	// Memory to back up the depth buffer
	vk::DeviceMemory depthBufferDeviceMemory;
	
	uint32_t graphicsQueueIndex = 0;
	uint32_t presentQueueIndex = 0;
	
	// The commandpool from which we allocate commandbuffers.
	vk::CommandPool graphicsCommandPool;
	std::vector<vk::CommandBuffer> commandBuffers;
	
	vk::DescriptorPool descriptorPool;
	
	std::vector<vk::ShaderModule> shaderModules;
	std::vector<vk::RenderPass> renderPasses;
};
