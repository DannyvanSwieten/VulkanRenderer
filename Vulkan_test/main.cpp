//
//  main.cpp
//  Vulkan_test
//
//  Created by Danny on 26/02/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#include <iostream>

#include "graphics_context.hpp"

int main(int argc, const char * argv[]) {
	
	Instance instance({"VK_LAYER_LUNARG_standard_validation"},{});
	auto& physicalDevices = instance.getPhysicalDevices();
	Device device(physicalDevices[0], instance);
	
	return 0;
}
