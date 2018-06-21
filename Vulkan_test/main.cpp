//
//  main.cpp
//  Vulkan_test
//
//  Created by Danny on 26/02/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#include "vulkan_renderer.hpp"
#include "window.hpp"

int main(int argc, const char * argv[]) {
	
	Window w(800, 600);
	DeviceRequirements requirements;
	requirements.graphicsQueueSupport = true;
	requirements.swapchainSupport = true;
	requirements.nativeWindowHandle = w.getNativeHandle();
	
	VulkanRenderer renderer(requirements);
	
	return 0;
}
