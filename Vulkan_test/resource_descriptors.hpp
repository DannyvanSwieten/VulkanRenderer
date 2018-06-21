//
//  resource_descriptors.hpp
//  metal_renderer
//
//  Created by Danny on 07/06/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#pragma once

#include <map>
#include <string>
#include <vector>

#include <experimental/optional>


using std::experimental::optional;

using resource_handle_t = std::size_t;
constexpr resource_handle_t null_handle = -1;

struct DeviceRequirements
{
	bool swapchainSupport 		= false;
	bool graphicsQueueSupport 	= false;
	
	void* nativeWindowHandle	= nullptr;
};

struct ClearColour
{
	float r;
	float g;
	float b;
	float a;
};

enum class DataType
{
	UNSIGNED_BYTE,
	BYTE,
	UNSIGNED_INT_16,
	INT_16,
	UNSIGNED_INT_32,
	INT_32,
	UNSIGNED_INT_64,
	INT_64,
	
	FLOAT_16,
	FLOAT_32,
	
	DOUBLE
};

enum class ChannelLayout
{
	R,
	RG,
	RGB,
	BGR,
	RGBA,
	BGRA,
	
	CO_CG_Y
};

enum class LoadAction
{
	NONE,
	LOAD,
	CLEAR
};

enum class TextureType
{
	ONE_DIMENSIONAL,
	TWO_DIMENSIONAL,
	THREE_DIMENSIONAL,
	
	CUBE,
	
	ARRAY_ONE_DIMENSIONAL,
	ARRAY_TWO_DIMENSIONAL,
	MULTI_SAMPLE_TWO_DIMENSIONAL
};

enum class TextureUsage
{
	READ,
	WRITE,
	RENDER_TARGET
};

enum class PrimitiveTopology
{
	TRIANGLES,
	LINES,
	POINTS
};

struct TextureDescriptor
{
	uint32_t width    = 0;
	uint32_t height   = 0;
	uint32_t depth    = 0;
	
	uint32_t samplesPerPixel = 1;
	
	ChannelLayout layout	= ChannelLayout::RGBA;
	DataType dataType    	= DataType::UNSIGNED_BYTE;
	
	TextureType type = TextureType::TWO_DIMENSIONAL;
};

struct RenderPassAttachmentDescriptor
{
	LoadAction loadAction;
	resource_handle_t texture = null_handle;
};

struct RenderPassColourAttachmentDescriptor: public RenderPassAttachmentDescriptor
{
	ClearColour clearColour;
};

struct RenderPassDepthAttachmentDescriptor: public RenderPassAttachmentDescriptor
{
	float clearDepth;
};

struct RenderPassDescriptor
{
	std::vector<RenderPassColourAttachmentDescriptor> colourAttachments;
	optional<RenderPassDepthAttachmentDescriptor> depthAttachment;
};

struct ShaderStageDescriptor
{
	enum class Type
	{
		VERTEX,
		FRAGMENT,
		GEOMETRY,
		TESSELLATION_CONTROL,
		TESSELATION_EVALUATION,
		COMPUTE
	};
	
	std::string entryPoint;
	Type type;
	resource_handle_t module;
};

struct VertexAttributeDescriptor
{
	DataType type;
	uint8_t numElements = 0;
	uint32_t offset 	= 0;
	uint32_t location 	= 0;
};

struct ViewPort
{
	float x			= 0;
	float y			= 0;
	
	float width 	= 0;
	float height 	= 0;
	
	float minDepth	= 0;
	float maxDepth	= 1;
};

struct DepthStencilStateDescriptor
{
	uint32_t write = 0;
	uint32_t test = 0;
	
	LoadAction loadAction;
};

struct RenderPipelineDescriptor
{
	std::vector<ViewPort> viewPorts;
	std::vector<ShaderStageDescriptor> shaderStages;
	std::vector<VertexAttributeDescriptor> vertexAttributeDescriptors;
	DepthStencilStateDescriptor depthStencilState;
	PrimitiveTopology topology;
	uint32_t primitiveRestart = 0;
	
	resource_handle_t renderPass		= null_handle;
};

struct NodeResourceDescriptor
{
	int32_t camera = -1;
	std::vector<int32_t> children;
	int32_t skin = -1;
	float matrix[16] {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	int32_t mesh = -1;
	
	float rotation[4] {0, 0, 0, 1};
	float scale[3] {1, 1, 1};
	float translation[3] {0, 0, 0};
	
	std::vector<float> weights;
	
	std::string name;
};

struct Primitive
{
	std::map<char*, int32_t> attributes;
	int32_t indices		= -1;
	int32_t material	= -1;
	int32_t mode		= 4;
};

struct Mesh
{
	std::vector<Primitive> primitives;
	std::vector<float> weights;
	std::string name;
};

struct BufferResourceDescriptor
{
	std::string name;
	std::string uri;
	int32_t byteLength 	= -1;
	void* data = nullptr;
};

struct BufferViewResourceDescriptor
{
	int32_t bufferId 	= -1;
	int32_t byteOffset 	= 0;
	int32_t byteLength	= -1;
	int32_t byteStride	= -1;
	int32_t target		= -1;
	
	std::string name;
};

struct ImageResourceDescriptor
{
	std::string uri;
	std::string mimeType;
	int32_t bufferView	= -1;
	std::string name;
};

struct SamplerResourceDescriptor
{
	int32_t magFilter 	= -1;
	int32_t minFilter 	= -1;
	int32_t wrapS		= 10497;
	int32_t wrapT		= 10497;
	std::string name;
};

struct Accessor
{
	
};
