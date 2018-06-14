//
//  window.cpp
//  Vulkan_test
//
//  Created by Danny on 16/03/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#include "metal_view.h"

#include "window.hpp"

struct Pimpl {
	int width;
	int height;
	
	NSWindow* window;
	MetalView* view;
};

Window::Window(int width, int height) { 
	pimpl = std::make_unique<Pimpl>();
	pimpl->window = [[NSWindow alloc] initWithContentRect: CGRectMake(0, 0, width, height)
												styleMask: 0
												  backing: NSBackingStoreBuffered
													defer: true];
	
	pimpl-> view = [[MetalView alloc] initWithFrame:CGRectMake(0, 0, width, height)];
	[pimpl->view setWantsLayer: true];
	[pimpl->window setContentView:pimpl->view];
}

Window::~Window() {
	
}

void* Window::getNativeHandle() {
	return (__bridge void*)pimpl->view;
}
