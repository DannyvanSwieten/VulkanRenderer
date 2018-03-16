//
//  window.hpp
//  Vulkan_test
//
//  Created by Danny on 16/03/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#pragma once

#include <memory>

struct Pimpl;

class Window {
public:
	Window(int width, int height);
	~Window();
	
	void* getNativeHandle();
	
private:
	
	std::unique_ptr<Pimpl> pimpl;
};
