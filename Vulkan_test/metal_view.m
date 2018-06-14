//
//  MetalView.m
//  Vulkan_test
//
//  Created by Danny on 16/03/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#import "metal_view.h"

#import <QuartzCore/CAMetalLayer.h>

@implementation MetalView
/** Indicates that the view wants to draw using the backing layer instead of using drawRect:.  */
-(BOOL) wantsUpdateLayer { return YES; }

/** Returns a Metal-compatible layer. */
+(Class) layerClass { return [CAMetalLayer class]; }

/** If the wantsLayer property is set to YES, this method will be invoked to return a layer instance. */
-(CALayer*) makeBackingLayer {
	CALayer* layer = [self.class.layerClass layer];
	CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
	layer.contentsScale = MIN(viewScale.width, viewScale.height);
	return layer;
}

-(BOOL) acceptsFirstResponder { return YES; }

@end
