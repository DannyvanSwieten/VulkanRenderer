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

VulkanRenderer::VulkanRenderer(const DeviceRequirements& reqs) {
	
	const std::vector<const char* const> validationLayers { "VK_LAYER_LUNARG_standard_validation" };
	std::vector<const char*> requiredExtensions;
	if(reqs.swapchainSupport)
	{
		requiredExtensions.emplace_back ( "VK_KHR_surface" );
	
#ifdef __APPLE__
	requiredExtensions.emplace_back("VK_MVK_macos_surface");
#endif
		
	}
	
	vk::InstanceCreateInfo info;
	info.setPpEnabledLayerNames(validationLayers.data()).
	setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size())).
	setPpEnabledExtensionNames(requiredExtensions.data()).
	setEnabledExtensionCount(static_cast<uint32_t>(requiredExtensions.size()));
	
	instance = vk::createInstance(info);
	chooseBestDevice(instance.enumeratePhysicalDevices(), reqs);
	if(reqs.graphicsQueueSupport)
		createLogicalDeviceAndPresentQueue(reqs);
	
	createSwapChain();
	createCommandPool();
	createDescriptorPool();
}

void VulkanRenderer::chooseBestDevice(const std::vector<vk::PhysicalDevice>& devices, const DeviceRequirements& reqs) {
	
	std::vector<vk::PhysicalDevice> suitableDevices;
	
	// Filter out devices that don't have platform specific surface extension;
	
	if(reqs.swapchainSupport) {
		for(const auto& device: devices) {
			if(checkSwapChainCompatibilityForDevice(device, reqs)) {
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
	
	if(suitableDevices.empty())
		return;
	
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

void VulkanRenderer::createPlatformSpecificSurface(void* nativeWindowHandle) {
#ifdef __APPLE__
	vk::MacOSSurfaceCreateInfoMVK surfaceCreateInfo;
	surfaceCreateInfo.pView = nativeWindowHandle;
	surface = instance.createMacOSSurfaceMVK(surfaceCreateInfo);
#endif
}

void VulkanRenderer::createLogicalDeviceAndPresentQueue(const DeviceRequirements& reqs) {
	
	auto layers 	= physicalDevice.enumerateDeviceLayerProperties();
	auto features 	= physicalDevice.getFeatures();
	auto extensions = physicalDevice.enumerateDeviceExtensionProperties();
	auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	
	uint32_t index = 0;
	for(auto& p: queueFamilyProperties) {
		if(p.queueCount > 0 && p.queueFlags & vk::QueueFlagBits::eGraphics) {
			graphicsQueueIndex = index;
		}
		
		if(reqs.swapchainSupport) {
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

bool VulkanRenderer::checkSwapChainCompatibilityForDevice(const vk::PhysicalDevice &device, const DeviceRequirements& reqs) {

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
		
		createPlatformSpecificSurface(reqs.nativeWindowHandle);
		
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
	
	// Create image views for all swapchain images
	for(const auto& image: swapChainImages) {
		
		vk::ComponentMapping mapping;
		
		vk::ImageSubresourceRange subResource;
		subResource.setAspectMask(vk::ImageAspectFlagBits::eColor);
		subResource.setLevelCount(1);
		subResource.setLayerCount(1);
		
		vk::ImageViewCreateInfo viewInfo;
		viewInfo.setImage(image);
		viewInfo.setFormat(swapChainFormat.format);
		viewInfo.setViewType(vk::ImageViewType::e2D);
		viewInfo.setComponents(mapping);
		viewInfo.setSubresourceRange(subResource);
		swapChainImageViews.emplace_back(logicalDevice.createImageView(viewInfo));
	}
	
	// Create depthbuffer
	vk::ImageCreateInfo depthBufferCreateInfo;
	depthBufferCreateInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
	depthBufferCreateInfo.setFormat(vk::Format::eD24UnormS8Uint);
	depthBufferCreateInfo.setImageType(vk::ImageType::e2D);
	const auto& windowSize = surfaceCababilities.currentExtent;
	depthBufferCreateInfo.setExtent(vk::Extent3D{windowSize.width, windowSize.height, 1});
	depthBufferCreateInfo.setMipLevels(1);
	depthBufferCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
	depthBufferCreateInfo.setArrayLayers(1);
	depthBufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
	
	depthBuffer = logicalDevice.createImage(depthBufferCreateInfo);
	
	// Allocate devicememory for depthbuffer.
	auto memoryProperties = physicalDevice.getMemoryProperties();
	const auto memoryRequirements = logicalDevice.getImageMemoryRequirements(depthBuffer);
	vk::MemoryAllocateInfo memoryInfo;
	memoryInfo.setAllocationSize(memoryRequirements.size);
	memoryInfo.setMemoryTypeIndex(memoryProperties.memoryTypes[0].heapIndex);
	depthBufferDeviceMemory = logicalDevice.allocateMemory(memoryInfo);
	
	// Bind it to the depthbuffer
	logicalDevice.bindImageMemory(depthBuffer, depthBufferDeviceMemory, 0);
	
	// Create imageview for depthbuffer
	vk::ImageSubresourceRange subResource;
	subResource.setAspectMask(vk::ImageAspectFlagBits::eDepth);
	subResource.setLevelCount(1);
	subResource.setLayerCount(1);
	
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.setImage(depthBuffer);
	viewInfo.setFormat(vk::Format::eD24UnormS8Uint);
	viewInfo.setViewType(vk::ImageViewType::e2D);
	viewInfo.setComponents(vk::ComponentMapping());
	viewInfo.setSubresourceRange(subResource);

	depthBufferView = logicalDevice.createImageView(viewInfo);
	
	vk::AttachmentDescription attachmentDescription;
	attachmentDescription.setLoadOp(vk::AttachmentLoadOp::eClear);
	attachmentDescription.setStoreOp(vk::AttachmentStoreOp::eStore);
	attachmentDescription.setSamples(vk::SampleCountFlagBits::e1);
	attachmentDescription.setInitialLayout(vk::ImageLayout::eUndefined);
	attachmentDescription.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	attachmentDescription.setFormat(vk::Format::eB8G8R8A8Unorm);
	
	vk::AttachmentReference attachmentRef;
	attachmentRef.setAttachment(0);
	attachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	
	vk::SubpassDescription subDescription;
	subDescription.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subDescription.setColorAttachmentCount(1);
	subDescription.setPColorAttachments(&attachmentRef);
	
	vk::RenderPassCreateInfo rpCreateInfo;
	rpCreateInfo.setSubpassCount(1);
	rpCreateInfo.setPSubpasses(&subDescription);
	rpCreateInfo.setAttachmentCount(1);
	rpCreateInfo.setPAttachments(&attachmentDescription);
	
	renderPass = logicalDevice.createRenderPass(rpCreateInfo);
	
	for(auto i = 0; i < swapChainImages.size(); ++i)
	{
		vk::FramebufferCreateInfo info;
		info.setLayers(1);
		info.setWidth(surfaceCababilities.currentExtent.width);
		info.setHeight(surfaceCababilities.currentExtent.height);
		info.setRenderPass(renderPass);
		info.setPAttachments(&swapChainImageViews[i]);
		info.setAttachmentCount(1);
		
		swapChainFrameBuffers.emplace_back(logicalDevice.createFramebuffer(info));
	}
	
	
}

void VulkanRenderer::createCommandPool() { 
	vk::CommandPoolCreateInfo poolInfo;
	poolInfo.setQueueFamilyIndex(graphicsQueueIndex);
	
	graphicsCommandPool = logicalDevice.createCommandPool(poolInfo);
	
	vk::CommandBufferAllocateInfo commandBufferInfo;
	commandBufferInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	commandBufferInfo.setCommandPool(graphicsCommandPool);
	commandBufferInfo.setCommandBufferCount(1);
	
	commandBuffers = logicalDevice.allocateCommandBuffers(commandBufferInfo);
}

void VulkanRenderer::createDescriptorPool() {
	std::vector<vk::DescriptorPoolSize> poolSizes;
	
	vk::DescriptorPoolSize size;
	size.setType(vk::DescriptorType::eUniformBuffer);
	size.setDescriptorCount(32);
	poolSizes.push_back(size);
	
	size.setType(vk::DescriptorType::eUniformBufferDynamic);
	size.setDescriptorCount(32);
	poolSizes.push_back(size);
	
	size.setType(vk::DescriptorType::eSampler);
	size.setDescriptorCount(32);
	poolSizes.push_back(size);
	
	size.setType(vk::DescriptorType::eSampledImage);
	size.setDescriptorCount(32);
	poolSizes.push_back(size);
	
	size.setType(vk::DescriptorType::eStorageImage);
	size.setDescriptorCount(32);
	poolSizes.push_back(size);
	
	size.setType(vk::DescriptorType::eStorageBuffer);
	size.setDescriptorCount(32);
	poolSizes.push_back(size);
	
	size.setType(vk::DescriptorType::eStorageBufferDynamic);
	size.setDescriptorCount(32);
	poolSizes.push_back(size);
	
	vk::DescriptorPoolCreateInfo info;
	info.setPPoolSizes(poolSizes.data());
	info.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()));
	info.setMaxSets(32);
	
	descriptorPool = logicalDevice.createDescriptorPool(info);
}

resource_handle_t VulkanRenderer::createShaderModule(const std::string& descriptor)
{
	// give source to SPIR-V compiler which outputs a uint32*
	std::vector<uint32_t> code;
	return createShaderModuleFromSpirV(code);
}

resource_handle_t VulkanRenderer::createShaderModuleFromSpirV(const std::vector<uint32_t> instructions)
{
	vk::ShaderModuleCreateInfo info;
	info.setCodeSize(instructions.size());
	info.setPCode(instructions.data());
	
	auto module = logicalDevice.createShaderModule(info);
	if(module)
		shaderModules.emplace_back(module);
	
	return shaderModules.size() - 1;
}

vk::VertexInputAttributeDescription VulkanRenderer::createAttributeDescription(const VertexAttributeDescriptor &attribute)
{
	vk::VertexInputAttributeDescription d;
	d.setOffset(attribute.offset);
	d.setLocation(attribute.location);
	
	switch(attribute.type)
	{
		case DataType::FLOAT_32:
		{
			switch(attribute.numElements)
			{
				case 1: d.setFormat(vk::Format::eR32Sfloat); break;
				case 2: d.setFormat(vk::Format::eR32G32Sfloat); break;
				case 3: d.setFormat(vk::Format::eR32G32B32Sfloat); break;
				case 4: d.setFormat(vk::Format::eR32G32B32A32Sfloat); break;
			}
		}
			
		case DataType::INT_32:
		{
			switch(attribute.numElements)
			{
				case 1: d.setFormat(vk::Format::eR32Sint); break;
				case 2: d.setFormat(vk::Format::eR32G32Sint); break;
				case 3: d.setFormat(vk::Format::eR32G32B32Sint); break;
				case 4: d.setFormat(vk::Format::eR32G32B32A32Sint); break;
			}
		}
			
		default: break;
	}
	
	return d;
}

resource_handle_t VulkanRenderer::createRenderPipeline(const RenderPipelineDescriptor &descriptor)
{
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	std::vector<vk::VertexInputAttributeDescription> vkAttributes;
	for(const auto& attribute: descriptor.vertexAttributeDescriptors)
		vkAttributes.emplace_back(createAttributeDescription(attribute));

	vertexInputInfo.setPVertexAttributeDescriptions(vkAttributes.data()).
	setVertexAttributeDescriptionCount(static_cast<uint32_t>(vkAttributes.size()));
	
	vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
	if(descriptor.primitiveRestart)
		assemblyInfo.setPrimitiveRestartEnable(true);
	
	switch(descriptor.topology)
	{
		case PrimitiveTopology::POINTS: assemblyInfo.setTopology(vk::PrimitiveTopology::ePointList); break;
		case PrimitiveTopology::LINES: assemblyInfo.setTopology(vk::PrimitiveTopology::eLineList); break;
		case PrimitiveTopology::TRIANGLES: assemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList); break;
	}
	
	vk::DescriptorSetLayoutBinding layoutBinding;
	layoutBinding.setDescriptorType(vk::DescriptorType::eStorageBuffer);
	layoutBinding.setBinding(0);
	
	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.setPBindings(&layoutBinding);
	layoutInfo.setBindingCount(0);
	
	auto descriptorSetLayout = logicalDevice.createDescriptorSetLayout(layoutInfo);
	
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setPSetLayouts(&descriptorSetLayout);
	pipelineLayoutInfo.setSetLayoutCount(1);
	
	auto pipelineLayout = logicalDevice.createPipelineLayout(pipelineLayoutInfo);
	
	vk::PipelineViewportStateCreateInfo vpInfo;
	
	std::vector<vk::Viewport> viewports;
	for(const auto& vp: descriptor.viewPorts)
		viewports.emplace_back(vp.x, vp.y, vp.width, vp.height, vp.minDepth, vp.maxDepth);
	
	vpInfo.setViewportCount(static_cast<uint32_t>(viewports.size()));
	vpInfo.setPViewports(viewports.data());
	
	vk::ClearDepthStencilValue clearDepth;
	clearDepth.setDepth(0);
	clearDepth.setStencil(0);
	
	vk::PipelineDepthStencilStateCreateInfo depthInfo;
	depthInfo.setDepthTestEnable(descriptor.depthStencilState.test);
	depthInfo.setDepthWriteEnable(descriptor.depthStencilState.write);
	
	std::vector<vk::PipelineShaderStageCreateInfo> stages;
	for(const auto& stage: descriptor.shaderStages)
	{
		vk::PipelineShaderStageCreateInfo stageInfo;
		stageInfo.setModule(shaderModules[stage.module]);
		stageInfo.setPName(stage.entryPoint.c_str());
		
		switch(stage.type)
		{
			case ShaderStageDescriptor::Type::VERTEX: stageInfo.setStage(vk::ShaderStageFlagBits::eVertex); break;
			case ShaderStageDescriptor::Type::FRAGMENT: stageInfo.setStage(vk::ShaderStageFlagBits::eFragment); break;
			case ShaderStageDescriptor::Type::GEOMETRY: stageInfo.setStage(vk::ShaderStageFlagBits::eGeometry); break;
			case ShaderStageDescriptor::Type::TESSELATION_EVALUATION: stageInfo.setStage(vk::ShaderStageFlagBits::eTessellationEvaluation); break;
			case ShaderStageDescriptor::Type::TESSELLATION_CONTROL: stageInfo.setStage(vk::ShaderStageFlagBits::eTessellationControl); break;
			case ShaderStageDescriptor::Type::COMPUTE: stageInfo.setStage(vk::ShaderStageFlagBits::eCompute); break;
		}
	}
	
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	
	vk::GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.setLayout(pipelineLayout);
	auto& rp = renderPasses.at(descriptor.renderPass);
	pipelineInfo.setRenderPass(rp);
	pipelineInfo.setSubpass(0);
	pipelineInfo.setPViewportState(&vpInfo);
	pipelineInfo.setPDepthStencilState(&depthInfo);
	pipelineInfo.setLayout(pipelineLayout);
	pipelineInfo.setPVertexInputState(&vertexInputInfo);
	pipelineInfo.setPInputAssemblyState(&assemblyInfo);
	pipelineInfo.setPStages(stages.data());
	pipelineInfo.setStageCount(static_cast<uint32_t>(stages.size()));
	
	return 0;
}

resource_handle_t VulkanRenderer::createRenderpass(const RenderPassDescriptor& descriptor)
{
	std::vector<vk::AttachmentDescription> vkAttachmentDescriptors;
	std::vector<vk::AttachmentReference> vkAttachmentRefs;
	uint32_t index = 0;
	for(const auto& attachment: descriptor.colourAttachments)
	{
		vk::AttachmentDescription desc;
		desc.setInitialLayout(vk::ImageLayout::eUndefined);
		desc.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
		desc.setSamples(vk::SampleCountFlagBits::e1);
		
		switch(attachment.loadAction)
		{
			case LoadAction::LOAD: desc.setLoadOp(vk::AttachmentLoadOp::eLoad);
			case LoadAction::CLEAR: desc.setLoadOp(vk::AttachmentLoadOp::eClear);
			default: desc.setLoadOp(vk::AttachmentLoadOp::eDontCare);
		}
		
		vkAttachmentDescriptors.emplace_back(desc);
		
		vk::AttachmentReference ref;
		ref.setAttachment(index);
		ref.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
		vkAttachmentRefs.emplace_back(ref);
		
		index++;
	}
	
	vk::SubpassDescription subpass;
	subpass.setColorAttachmentCount(static_cast<uint32_t>(descriptor.colourAttachments.size()));
	subpass.setPColorAttachments(vkAttachmentRefs.data());
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	
	if(descriptor.depthAttachment)
	{
		vk::AttachmentDescription desc;
		desc.setInitialLayout(vk::ImageLayout::eUndefined);
		desc.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		desc.setLoadOp(vk::AttachmentLoadOp::eClear);
		
		vk::AttachmentReference depthRef;
		depthRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		depthRef.setAttachment(index);
		
		subpass.setPDepthStencilAttachment(&depthRef);
		subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
		
		vkAttachmentDescriptors.emplace_back(desc);
		vkAttachmentRefs.emplace_back(depthRef);
	}
	
	vk::RenderPassCreateInfo info;
	info.setAttachmentCount(static_cast<uint32_t>(vkAttachmentDescriptors.size()));
	info.setPAttachments(vkAttachmentDescriptors.data());
	info.setSubpassCount(1);
	info.setPSubpasses(&subpass);
	
	auto renderpass = logicalDevice.createRenderPass(info);
	if(!renderpass)
		return -1;
	
	renderPasses.emplace_back(renderpass);
	return renderPasses.size() - 1;
}
