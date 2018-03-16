//
//  vulkan_renderer.cpp
//  Vulkan_test
//
//  Created by Danny on 15/03/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#ifdef __APPLE__
#define VK_USE_PLATFORM_MACOS_MVK
#endif

#include "vulkan_renderer.hpp"

#include <map>

VulkanRenderer::VulkanRenderer(void* nativeWindowHandle): nativeWindowHandle(nativeWindowHandle) {
	
	const std::vector<const char* const> validationLayers { "VK_LAYER_LUNARG_standard_validation" };
	const std::vector<const char* const> requiredExtensions { "VK_KHR_surface", "VK_MVK_macos_surface" };
	vk::InstanceCreateInfo info;
	info.setPpEnabledLayerNames(validationLayers.data()).
	setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size())).
	setPpEnabledExtensionNames(requiredExtensions.data()).
	setEnabledExtensionCount(static_cast<uint32_t>(requiredExtensions.size()));
	
	instance = vk::createInstance(info);
	chooseBestDevice(instance.enumeratePhysicalDevices());
	createLogicalDeviceAndPresentQueue();
	createSwapChain();
}

void VulkanRenderer::chooseBestDevice(const std::vector<vk::PhysicalDevice>& devices) {
	
	std::vector<vk::PhysicalDevice> suitableDevices;
	
	// Filter out devices that don't have platform specific surface extension;
	
	if(nativeWindowHandle) {
		createPlatformSpecificSurface();
		
		for(const auto& device: devices) {
			if(checkSwapChainCompatibilityForDevice(device)) {
				suitableDevices.emplace_back(device);
			}
		}

	} else {
		// Filter out devices that don't have a graphics queue
		for(const auto& device: devices) {
			for(auto& property: device.getQueueFamilyProperties()) {
				if(property.queueFlags & vk::QueueFlagBits::eGraphics && property.queueCount > 0) {
					suitableDevices.emplace_back(device);
				}
			}
		}
	}
	
	size_t maxScore = 0;
	size_t deviceId = 0;
	size_t winner = 0;
	
	for(const auto& device: suitableDevices) {
		size_t localScore = 0;
		
		const auto limits = device.getProperties().limits;
		localScore += (unsigned int)limits.framebufferColorSampleCounts;
		localScore += (unsigned int)limits.framebufferDepthSampleCounts;
		localScore += (unsigned int)limits.maxImageDimension1D;
		localScore += (unsigned int)limits.maxImageDimension2D;
		localScore += (unsigned int)limits.maxImageDimension3D;
		
		if(localScore > maxScore) {
			maxScore = localScore;
			winner = deviceId;
		}
		
		deviceId++;
	}
	
	physicalDevice = suitableDevices[winner];
}

void VulkanRenderer::createPlatformSpecificSurface() {
#ifdef __APPLE__
	vk::MacOSSurfaceCreateInfoMVK surfaceCreateInfo;
	surfaceCreateInfo.pView = nativeWindowHandle;
	surface = instance.createMacOSSurfaceMVK(surfaceCreateInfo);
#endif
}

void VulkanRenderer::createLogicalDeviceAndPresentQueue() {
	
	auto layers 	= physicalDevice.enumerateDeviceLayerProperties();
	auto features 	= physicalDevice.getFeatures();
	auto extensions = physicalDevice.enumerateDeviceExtensionProperties();
	auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	
	uint32_t index = 0;
	for(auto& p: queueFamilyProperties) {
		if(p.queueCount > 0 && p.queueFlags & vk::QueueFlagBits::eGraphics) {
			graphicsQueueIndex = index;
		}
		
		if(surface) {
			vk::Bool32 presentSupportForSurface = false;
			physicalDevice.getSurfaceSupportKHR(index, surface, &presentSupportForSurface);
			if(presentSupportForSurface)
				presentQueueIndex = index;
		}
			
		index++;
	}
	
	std::vector<const char*> extensionNames;
	extensionNames.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	
	std::vector<const char*> layerNames;
	for(auto& l: layers)
		layerNames.emplace_back(l.layerName);
	
	float queuePriority = 1.0;
	vk::DeviceQueueCreateInfo queueInfo;
	queueInfo.setQueueCount(1).
	setPQueuePriorities(&queuePriority).
	setQueueFamilyIndex(graphicsQueueIndex);
	
	vk::DeviceCreateInfo logicalDeviceCreateInfo;
	logicalDeviceCreateInfo.setPEnabledFeatures(&features).
	setEnabledExtensionCount(static_cast<uint32_t>(extensionNames.size())).
	setPpEnabledExtensionNames(extensionNames.data()).
	setEnabledLayerCount(static_cast<uint32_t>(layerNames.size())).
	setPpEnabledLayerNames(layerNames.data()).
	setQueueCreateInfoCount(1).
	setPQueueCreateInfos(&queueInfo);
	
	logicalDevice = physicalDevice.createDevice(logicalDeviceCreateInfo);
	presentQueue = logicalDevice.getQueue(graphicsQueueIndex, 0);
	
	if(surface) {
		surfaceCababilities 	= physicalDevice.getSurfaceCapabilitiesKHR(surface);
		supportedSurfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
		supportedPresentModes 	= physicalDevice.getSurfacePresentModesKHR(surface);
		chooseSurfaceFormatForSwapChain();
		choosePresentModeForSwapChain();
	}
}

bool VulkanRenderer::checkSwapChainCompatibilityForDevice(const vk::PhysicalDevice &device) { 

	std::string surfaceExtension;
#ifdef __APPLE__
	surfaceExtension = "VK_MVK_macos_surface";
#endif
	
	bool compatible = false;
	auto extensionProperties = device.enumerateDeviceExtensionProperties();
	for(auto& extensionProperty: extensionProperties) {
		if(extensionProperty.extensionName == surfaceExtension) {
			compatible = true;
		}
	}
	
	if(compatible) {
		uint32_t queueIndex = 0;
		for(auto& property: device.getQueueFamilyProperties()) {
			if(property.queueFlags & vk::QueueFlagBits::eGraphics && property.queueCount > 0) {
				vk::Bool32 presentSupportForSurface = false;
				device.getSurfaceSupportKHR(queueIndex, surface, &presentSupportForSurface);
				if(presentSupportForSurface)
					return true;
			}
			
			queueIndex++;
		}
	}
	
	return false;
}

void VulkanRenderer::chooseSurfaceFormatForSwapChain() { 
	if(supportedSurfaceFormats.size() == 1 && supportedSurfaceFormats[0].format == vk::Format::eUndefined) {
		swapChainFormat = {vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
		return;
	}
	
	for(const auto& format: supportedSurfaceFormats) {
		if(format.format == vk::Format::eR8G8B8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			swapChainFormat = format;
			return;
		}
	}
	
	swapChainFormat = supportedSurfaceFormats[0];
}

void VulkanRenderer::choosePresentModeForSwapChain() { 
	vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo;
	
	for(const auto& pMode: supportedPresentModes) {
		if(pMode == vk::PresentModeKHR::eMailbox) {
			swapChainPresentMode = mode;
			return;
		} else if (pMode == vk::PresentModeKHR::eImmediate) {
			mode = pMode;
		}
	}
	
	swapChainPresentMode = mode;
}

void VulkanRenderer::createSwapChain() { 
	vk::SwapchainCreateInfoKHR swapChainInfo;
	swapChainInfo.setSurface(surface);
	swapChainInfo.setImageFormat(swapChainFormat.format);
	swapChainInfo.setMinImageCount(surfaceCababilities.minImageCount);
	swapChainInfo.setImageExtent(surfaceCababilities.currentExtent);
	swapChainInfo.setPresentMode(swapChainPresentMode);
	swapChainInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	swapChainInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
	swapChainInfo.setImageArrayLayers(1);
	
	swapChain = logicalDevice.createSwapchainKHR(swapChainInfo);
	swapChainImages = logicalDevice.getSwapchainImagesKHR(swapChain);
	
	for(const auto& image: swapChainImages) {
		
		vk::ComponentMapping mapping;
		mapping.setR(vk::ComponentSwizzle::eB);
		mapping.setB(vk::ComponentSwizzle::eR);
		
		vk::ImageViewCreateInfo viewInfo;
		viewInfo.setImage(image);
		viewInfo.setFormat(swapChainFormat.format);
		viewInfo.setViewType(vk::ImageViewType::e2D);
		viewInfo.setComponents(mapping);
		swapChainImageViews.emplace_back(logicalDevice.createImageView(viewInfo));
	}
	
	vk::ImageCreateInfo depthBufferCreateInfo;
	depthBufferCreateInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
	depthBufferCreateInfo.setFormat(vk::Format::eD16Unorm);
	const auto& windowSize = surfaceCababilities.currentExtent;
	depthBufferCreateInfo.setExtent(vk::Extent3D{windowSize.width, windowSize.height, 1});
	depthBufferCreateInfo.setMipLevels(1);
	depthBufferCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
	depthBufferCreateInfo.setArrayLayers(1);
	depthBufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
}




