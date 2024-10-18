/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_util.hpp"
#include "gl_api.hpp"
#include <cassert>
#include <stdexcept>

using namespace prosper;

GLuint prosper::util::to_opengl_enum(prosper::Filter filter)
{
	auto glFilter = GL_LINEAR;
	switch(filter) {
	case prosper::Filter::Linear:
		glFilter = GL_LINEAR;
		break;
	case prosper::Filter::Nearest:
		glFilter = GL_NEAREST;
		break;
	}
	return glFilter;
}

GLuint prosper::util::to_opengl_enum(prosper::SamplerAddressMode addressMode)
{
	switch(addressMode) {
	case SamplerAddressMode::Repeat:
		return GL_REPEAT;
	case SamplerAddressMode::MirroredRepeat:
		return GL_MIRRORED_REPEAT;
	case SamplerAddressMode::ClampToEdge:
		return GL_CLAMP_TO_EDGE;
	case SamplerAddressMode::ClampToBorder:
		return GL_CLAMP_TO_BORDER;
	case SamplerAddressMode::MirrorClampToEdge:
		return GL_MIRROR_CLAMP_TO_EDGE;
	}
	return GL_REPEAT;
}

GLuint prosper::util::to_opengl_enum(prosper::CompareOp compareOp)
{
	switch(compareOp) {
	case CompareOp::Never:
		return GL_NEVER;
	case CompareOp::Less:
		return GL_LESS;
	case CompareOp::Equal:
		return GL_EQUAL;
	case CompareOp::LessOrEqual:
		return GL_LEQUAL;
	case CompareOp::Greater:
		return GL_GREATER;
	case CompareOp::NotEqual:
		return GL_NOTEQUAL;
	case CompareOp::GreaterOrEqual:
		return GL_GEQUAL;
	case CompareOp::Always:
		return GL_ALWAYS;
	}
	return GL_NONE;
}

GLenum prosper::util::to_opengl_enum(prosper::PrimitiveTopology primitiveTopology)
{
	switch(primitiveTopology) {
	case prosper::PrimitiveTopology::PointList:
		return GL_POINTS;
	case prosper::PrimitiveTopology::LineList:
		return GL_LINES;
	case prosper::PrimitiveTopology::LineStrip:
		return GL_LINE_STRIP;
	case prosper::PrimitiveTopology::TriangleList:
		return GL_TRIANGLES;
	case prosper::PrimitiveTopology::TriangleStrip:
		return GL_TRIANGLE_STRIP;
	case prosper::PrimitiveTopology::TriangleFan:
		return GL_TRIANGLE_FAN;
	case prosper::PrimitiveTopology::LineListWithAdjacency:
		return GL_LINES_ADJACENCY;
	case prosper::PrimitiveTopology::LineStripWithAdjacency:
		return GL_LINE_STRIP_ADJACENCY;
	case prosper::PrimitiveTopology::TriangleListWithAdjacency:
		return GL_TRIANGLES_ADJACENCY;
	case prosper::PrimitiveTopology::TriangleStripWithAdjacency:
		return GL_TRIANGLE_STRIP_ADJACENCY;
	case prosper::PrimitiveTopology::PatchList:
		return GL_PATCHES;
	}
	return GL_NONE;
}

GLenum prosper::util::to_opengl_enum(prosper::BlendOp blendOp)
{
	switch(blendOp) {
	case BlendOp::Add:
		return GL_FUNC_ADD;
	case BlendOp::Subtract:
		return GL_FUNC_SUBTRACT;
	case BlendOp::ReverseSubtract:
		return GL_FUNC_REVERSE_SUBTRACT;
	case BlendOp::Min:
		return GL_MIN;
	case BlendOp::Max:
		return GL_MAX;
	}
	assert(false);
	return 0;
}

GLenum prosper::util::to_opengl_enum(prosper::BlendFactor blendFactor)
{
	switch(blendFactor) {
	case BlendFactor::Zero:
		return GL_ZERO;
	case BlendFactor::One:
		return GL_ONE;
	case BlendFactor::SrcColor:
		return GL_SRC_COLOR;
	case BlendFactor::OneMinusSrcColor:
		return GL_ONE_MINUS_SRC_COLOR;
	case BlendFactor::DstColor:
		return GL_DST_COLOR;
	case BlendFactor::OneMinusDstColor:
		return GL_ONE_MINUS_DST_COLOR;
	case BlendFactor::SrcAlpha:
		return GL_SRC_ALPHA;
	case BlendFactor::OneMinusSrcAlpha:
		return GL_ONE_MINUS_SRC_ALPHA;
	case BlendFactor::DstAlpha:
		return GL_DST_ALPHA;
	case BlendFactor::OneMinusDstAlpha:
		return GL_ONE_MINUS_DST_ALPHA;
	case BlendFactor::ConstantColor:
		return GL_CONSTANT_COLOR;
	case BlendFactor::OneMinusConstantColor:
		return GL_ONE_MINUS_CONSTANT_COLOR;
	case BlendFactor::ConstantAlpha:
		return GL_CONSTANT_ALPHA;
	case BlendFactor::OneMinusConstantAlpha:
		return GL_ONE_MINUS_CONSTANT_ALPHA;
	case BlendFactor::SrcAlphaSaturate:
		return GL_SRC_ALPHA_SATURATE;
	case BlendFactor::Src1Color:
		return GL_SRC1_COLOR;
	case BlendFactor::OneMinusSrc1Color:
		return GL_ONE_MINUS_SRC1_COLOR;
	case BlendFactor::Src1Alpha:
		return GL_SRC1_ALPHA;
	case BlendFactor::OneMinusSrc1Alpha:
		return GL_ONE_MINUS_SRC1_ALPHA;
	}
	assert(false);
	return 0;
}

GLenum prosper::util::to_opengl_enum(prosper::IndexType indexType)
{
	switch(indexType) {
	case IndexType::UInt16:
		return GL_UNSIGNED_SHORT;
	case IndexType::UInt32:
		return GL_UNSIGNED_INT;
	}
	assert(false);
	return 0;
}

GLenum prosper::util::to_opengl_image_format_type(prosper::Format format, GLboolean &outNormalized)
{
	outNormalized = GL_FALSE;
	switch(format) {
	case prosper::Format::A8B8G8R8_UNorm_Pack32:
		outNormalized = GL_TRUE;
		return GL_UNSIGNED_INT_8_8_8_8_REV;
	case prosper::Format::R8_UNorm:
	case prosper::Format::R8G8_UNorm:
	case prosper::Format::R8G8B8_UNorm_PoorCoverage:
	case prosper::Format::B8G8R8_UNorm_PoorCoverage:
	case prosper::Format::R8G8B8A8_UNorm:
	case prosper::Format::B8G8R8A8_UNorm:
		outNormalized = GL_TRUE;
	case prosper::Format::R8_UInt:
	case prosper::Format::R8G8_UInt:
	case prosper::Format::R8G8B8_UInt_PoorCoverage:
	case prosper::Format::R8G8B8A8_UInt:
		return GL_UNSIGNED_BYTE;
	case prosper::Format::A8B8G8R8_SNorm_Pack32:
		outNormalized = GL_TRUE;
		return GL_UNSIGNED_INT_8_8_8_8_REV;
	case prosper::Format::R8_SNorm:
	case prosper::Format::R8G8_SNorm:
	case prosper::Format::R8G8B8A8_SNorm:
		outNormalized = GL_TRUE;
	case prosper::Format::B8G8R8_SInt_PoorCoverage:
	case prosper::Format::B8G8R8A8_SInt:
	case prosper::Format::R8_SInt:
	case prosper::Format::R8G8_SInt:
	case prosper::Format::R8G8B8_SInt_PoorCoverage:
	case prosper::Format::R8G8B8A8_SInt:
		return GL_BYTE;
	case prosper::Format::R16_UNorm:
	case prosper::Format::R16G16_UNorm:
	case prosper::Format::R16G16B16_UNorm_PoorCoverage:
	case prosper::Format::R16G16B16A16_UNorm:
	case prosper::Format::D16_UNorm:
		outNormalized = GL_TRUE;
	case prosper::Format::R16_UInt:
	case prosper::Format::R16G16_UInt:
	case prosper::Format::R16G16B16_UInt_PoorCoverage:
	case prosper::Format::R16G16B16A16_UInt:
		return GL_UNSIGNED_SHORT;
	case prosper::Format::R16_SNorm:
	case prosper::Format::R16G16B16_SNorm_PoorCoverage:
		outNormalized = GL_TRUE;
	case prosper::Format::R16_SInt:
	case prosper::Format::R16G16_SInt:
	case prosper::Format::R16G16B16_SInt_PoorCoverage:
	case prosper::Format::R16G16B16A16_SInt:
		return GL_SHORT;
	case prosper::Format::R32_SInt:
	case prosper::Format::R32G32_SInt:
	case prosper::Format::R32G32B32_SInt:
	case prosper::Format::R32G32B32A32_SInt:
		return GL_INT;
	case prosper::Format::R32_UInt:
	case prosper::Format::R32G32_UInt:
	case prosper::Format::R32G32B32_UInt:
	case prosper::Format::R32G32B32A32_UInt:
		return GL_UNSIGNED_INT;
	case prosper::Format::R32_SFloat:
	case prosper::Format::R32G32_SFloat:
	case prosper::Format::R32G32B32_SFloat:
	case prosper::Format::R32G32B32A32_SFloat:
	case prosper::Format::D32_SFloat:
		return GL_FLOAT;
	case prosper::Format::R16_SFloat:
	case prosper::Format::R16G16_SFloat:
	case prosper::Format::R16G16B16_SFloat_PoorCoverage:
	case prosper::Format::R16G16B16A16_SFloat:
		return GL_HALF_FLOAT;
	case prosper::Format::R64_SFloat_PoorCoverage:
	case prosper::Format::R64G64_SFloat_PoorCoverage:
	case prosper::Format::R64G64B64_SFloat_PoorCoverage:
	case prosper::Format::R64G64B64A64_SFloat_PoorCoverage:
		return GL_DOUBLE;
	}
	throw std::logic_error {"Unsupported format!"};
}

GLenum prosper::util::to_opengl_image_format(prosper::Format format, GLenum *optOutPixelDataFormat)
{
	switch(format) {
	case prosper::Format::R8_UNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED;
		return GL_R8;
	case prosper::Format::R8_SNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED;
		return GL_R8_SNORM;
	case prosper::Format::R8_SRGB:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_SRGB8;
	case prosper::Format::R8G8_UNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG;
		return GL_RG8;
	case prosper::Format::R8G8B8_UNorm_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_RGB8;
	case prosper::Format::B8G8R8_UNorm_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_BGR;
		return GL_RGB8;
	case prosper::Format::R8G8B8A8_UNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_RGBA8;
	case prosper::Format::B8G8R8A8_UNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_BGRA;
		return GL_RGBA8;
	case prosper::Format::R16_UNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED;
		return GL_R16;
	case prosper::Format::R16_SNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED;
		return GL_R16_SNORM;
		;
	case prosper::Format::R16G16_UNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG;
		return GL_RG16;
	case prosper::Format::R16G16B16_UNorm_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_RGB16;
	case prosper::Format::R16G16B16A16_UNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_RGBA16;

	case prosper::Format::R8G8_SNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG;
		return GL_RG8_SNORM;

		//case prosper::Format::B8G8R8A8_UNorm:
		//	return GL_B8G8R8A8_UNorm;

	case prosper::Format::R32_SFloat:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED;
		return GL_R32F;
	case prosper::Format::R32G32_SFloat:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG;
		return GL_RG32F;
	case prosper::Format::R32G32B32_SFloat:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_RGB32F;
	case prosper::Format::R32G32B32A32_SFloat:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_RGBA32F;
	case prosper::Format::R8G8B8A8_SNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_RGBA8_SNORM;
	case prosper::Format::B8G8R8_SInt_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_BGR_INTEGER;
		return GL_BGR_INTEGER;
	case prosper::Format::B8G8R8A8_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_BGRA_INTEGER;
		return GL_BGRA_INTEGER;
	case prosper::Format::R4G4B4A4_UNorm_Pack16:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
	case prosper::Format::R4G4_UNorm_Pack8:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG;
		return GL_RG8; // TODO
	case prosper::Format::R5G6B5_UNorm_Pack16:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_RGB5;
	case prosper::Format::B5G6R5_UNorm_Pack16:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_BGR;
		return GL_RGB5;
	case prosper::Format::R5G5B5A1_UNorm_Pack16:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_RGB5;
	case prosper::Format::R8_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED_INTEGER;
		return GL_R8I;
	case prosper::Format::R8G8_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG_INTEGER;
		return GL_RG8I;
	case prosper::Format::R8G8B8_SInt_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB_INTEGER;
		return GL_RGB8I;
	case prosper::Format::R8G8B8A8_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA_INTEGER;
		return GL_RGBA8I;
	case prosper::Format::R8_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED_INTEGER;
		return GL_R8UI;
	case prosper::Format::R8G8_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG_INTEGER;
		return GL_RG8UI;
	case prosper::Format::R8G8B8_UInt_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB_INTEGER;
		return GL_RGB8UI;
	case prosper::Format::R8G8B8A8_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA_INTEGER;
		return GL_RGBA8UI;
	case prosper::Format::A8B8G8R8_UNorm_Pack32:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_ABGR_EXT;
		return GL_RGBA8;
	case prosper::Format::A8B8G8R8_SNorm_Pack32:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_ABGR_EXT;
		return GL_RGBA8_SNORM;
	case prosper::Format::R16_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED_INTEGER;
		return GL_R16I;
	case prosper::Format::R16G16_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG_INTEGER;
		return GL_RG16I;
	case prosper::Format::R16G16B16_SInt_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB_INTEGER;
		return GL_RGB16I;
	case prosper::Format::R16G16B16A16_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA_INTEGER;
		return GL_RGBA16I;
	case prosper::Format::R16_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED_INTEGER;
		return GL_R16UI;
	case prosper::Format::R16G16_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG_INTEGER;
		return GL_RG16UI;
	case prosper::Format::R16G16B16_UInt_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB_INTEGER;
		return GL_RGB16UI;
	case prosper::Format::R16G16B16A16_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA_INTEGER;
		return GL_RGBA16UI;
	case prosper::Format::R32_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED_INTEGER;
		return GL_R32I;
	case prosper::Format::R32G32_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG_INTEGER;
		return GL_RG32I;
	case prosper::Format::R32G32B32_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB_INTEGER;
		return GL_RGB32I;
	case prosper::Format::R32G32B32A32_SInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA_INTEGER;
		return GL_RGBA32I;
	case prosper::Format::R32_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED_INTEGER;
		return GL_R32UI;
	case prosper::Format::R32G32_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG_INTEGER;
		return GL_RG32UI;
	case prosper::Format::R32G32B32_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB_INTEGER;
		return GL_RGB32UI;
	case prosper::Format::R32G32B32A32_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA_INTEGER;
		return GL_RGBA32UI;
	case prosper::Format::R16_SFloat:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED;
		return GL_R16F;
	case prosper::Format::R16G16_SFloat:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG;
		return GL_RG16F;
	case prosper::Format::R16G16B16_SFloat_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_RGB16F;
	case prosper::Format::R16G16B16A16_SFloat:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_RGBA16F;
	case prosper::Format::R16G16B16_SNorm_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_RGB16_SNORM;
	case prosper::Format::R8G8B8A8_SRGB:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_SRGB8_ALPHA8;
	case prosper::Format::R8G8B8_SRGB_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_SRGB8;
	case prosper::Format::D16_UNorm:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_DEPTH_COMPONENT;
		return GL_DEPTH_COMPONENT16;
	case prosper::Format::D24_UNorm_S8_UInt_PoorCoverage:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_DEPTH_COMPONENT;
		return GL_DEPTH_COMPONENT24;
	case prosper::Format::D32_SFloat:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_DEPTH_COMPONENT;
		return GL_DEPTH_COMPONENT32F;
	case prosper::Format::D32_SFloat_S8_UInt:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_DEPTH_COMPONENT;
		return GL_DEPTH32F_STENCIL8;
	case prosper::Format::BC1_RGB_UNorm_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	case prosper::Format::BC1_RGB_SRGB_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
	case prosper::Format::BC1_RGBA_UNorm_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	case prosper::Format::BC1_RGBA_SRGB_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
	case prosper::Format::BC2_UNorm_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case prosper::Format::BC2_SRGB_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
	case prosper::Format::BC3_UNorm_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case prosper::Format::BC3_SRGB_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;

	case prosper::Format::BC4_UNorm_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED;
		return GL_COMPRESSED_RED_RGTC1;
	case prosper::Format::BC4_SNorm_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RED;
		return GL_COMPRESSED_SIGNED_RED_RGTC1;
	case prosper::Format::BC5_UNorm_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG;
		return GL_COMPRESSED_RG_RGTC2;
	case prosper::Format::BC5_SNorm_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RG;
		return GL_COMPRESSED_SIGNED_RG_RGTC2;
	case prosper::Format::BC6H_UFloat_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
	case prosper::Format::BC6H_SFloat_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
	case prosper::Format::BC7_UNorm_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGBA;
		return GL_COMPRESSED_RGBA_BPTC_UNORM;
	case prosper::Format::BC7_SRGB_Block:
		if(optOutPixelDataFormat)
			*optOutPixelDataFormat = GL_RGB;
		return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
	}
	throw std::logic_error {"Unsupported format!"};
	// TODO: Add ALL supported formats!
#if 0
	enum class Format : uint32_t
	{
		// Commented formats have poor coverage (<90%, see https://vulkan.gpuinfo.org/listformats.php)
		Unknown = 0,
		B4G4R4A4_UNorm_Pack16 = 3,
		= 4,
		= 5,
		R5G5B5A1_UNorm_Pack16 = 6,
		B5G5R5A1_UNorm_Pack16 = 7,
		A1R5G5B5_UNorm_Pack16 = 8,
		= 10,
		R8_UScaled_PoorCoverage = 11,
		R8_SScaled_PoorCoverage = 12,
		= 13,
		= 15,
		= 17,
		R8G8_UScaled_PoorCoverage = 18,
		R8G8_SScaled_PoorCoverage = 19,
		R8G8_SRGB_PoorCoverage = 22,
		R8G8B8_UNorm_PoorCoverage = 23,
		R8G8B8_SNorm_PoorCoverage = 24,
		R8G8B8_UScaled_PoorCoverage = 25,
		R8G8B8_SScaled_PoorCoverage = 26,
		R8G8B8_UInt_PoorCoverage = 27,
		R8G8B8_SInt_PoorCoverage = 28,
		R8G8B8_SRGB_PoorCoverage = 29,
		B8G8R8_UNorm_PoorCoverage = 30,
		B8G8R8_SNorm_PoorCoverage = 31,
		B8G8R8_UScaled_PoorCoverage = 32,
		B8G8R8_SScaled_PoorCoverage = 33,
		B8G8R8_UInt_PoorCoverage = 34,
		B8G8R8_SInt_PoorCoverage = 35,
		B8G8R8_SRGB_PoorCoverage = 36, // Coverage unknown
		= 38,
		R8G8B8A8_UScaled_PoorCoverage = 39,
		R8G8B8A8_SScaled_PoorCoverage = 40,
		= 44,
		B8G8R8A8_SNorm = 45,
		B8G8R8A8_UScaled_PoorCoverage = 46,
		B8G8R8A8_SScaled_PoorCoverage = 47,
		B8G8R8A8_UInt = 48,
		B8G8R8A8_SInt = 49,
		B8G8R8A8_SRGB = 50,
		A8B8G8R8_UNorm_Pack32 = 51,
		A8B8G8R8_SNorm_Pack32 = 52,
		A8B8G8R8_UScaled_Pack32_PoorCoverage = 53,
		A8B8G8R8_SScaled_Pack32_PoorCoverage = 54,
		A8B8G8R8_UInt_Pack32 = 55,
		A8B8G8R8_SInt_Pack32 = 56,
		A8B8G8R8_SRGB_Pack32 = 57,
		A2R10G10B10_UNorm_Pack32 = 58,
		A2R10G10B10_SNorm_Pack32_PoorCoverage = 59,
		A2R10G10B10_UScaled_Pack32_PoorCoverage = 60,
		A2R10G10B10_SScaled_Pack32_PoorCoverage = 61,
		A2R10G10B10_UInt_Pack32 = 62,
		A2R10G10B10_SInt_Pack32_PoorCoverage = 63,
		A2B10G10R10_UNorm_Pack32 = 64,
		A2B10G10R10_SNorm_Pack32_PoorCoverage = 65,
		A2B10G10R10_UScaled_Pack32_PoorCoverage = 66,
		A2B10G10R10_SScaled_Pack32_PoorCoverage = 67,
		A2B10G10R10_UInt_Pack32 = 68,
		A2B10G10R10_SInt_Pack32_PoorCoverage = 69,
		= 71,
		R16_UScaled_PoorCoverage = 72,
		R16_SScaled_PoorCoverage = 73,
		R16G16_SNorm = 78,
		R16G16_UScaled_PoorCoverage = 79,
		R16G16_SScaled_PoorCoverage = 80,
		R16G16B16_UNorm_PoorCoverage = 84,
		R16G16B16_SNorm_PoorCoverage = 85,
		R16G16B16_UScaled_PoorCoverage = 86,
		R16G16B16_SScaled_PoorCoverage = 87,
		R16G16B16_UInt_PoorCoverage = 88,
		R16G16B16_SInt_PoorCoverage = 89,
		R16G16B16_SFloat_PoorCoverage = 90,
		R16G16B16A16_SNorm = 92,
		R16G16B16A16_UScaled_PoorCoverage = 93,
		R16G16B16A16_SScaled_PoorCoverage = 94,
		R64_UInt_PoorCoverage = 110,
		R64_SInt_PoorCoverage = 111,
		R64_SFloat_PoorCoverage = 112,
		R64G64_UInt_PoorCoverage = 113,
		R64G64_SInt_PoorCoverage = 114,
		R64G64_SFloat_PoorCoverage = 115,
		R64G64B64_UInt_PoorCoverage = 116,
		R64G64B64_SInt_PoorCoverage = 117,
		R64G64B64_SFloat_PoorCoverage = 118,
		R64G64B64A64_UInt_PoorCoverage = 119,
		R64G64B64A64_SInt_PoorCoverage = 120,
		R64G64B64A64_SFloat_PoorCoverage = 121,
		B10G11R11_UFloat_Pack32 = 122,
		E5B9G9R9_UFloat_Pack32 = 123,
		X8_D24_UNorm_Pack32_PoorCoverage = 125,
		S8_UInt_PoorCoverage = 127,
		D16_UNorm_S8_UInt_PoorCoverage = 128,
		D24_UNorm_S8_UInt_PoorCoverage = 129,
		D32_SFloat_S8_UInt = 130,
		BC4_UNorm_Block = 139,
		BC4_SNorm_Block = 140,
		BC5_UNorm_Block = 141,
		BC5_SNorm_Block = 142,
		BC6H_UFloat_Block = 143,
		BC6H_SFloat_Block = 144,
		BC7_UNorm_Block = 145,
		BC7_SRGB_Block = 146,
		ETC2_R8G8B8_UNorm_Block_PoorCoverage = 147,
		ETC2_R8G8B8_SRGB_Block_PoorCoverage = 148,
		ETC2_R8G8B8A1_UNorm_Block_PoorCoverage = 149,
		ETC2_R8G8B8A1_SRGB_Block_PoorCoverage = 150,
		ETC2_R8G8B8A8_UNorm_Block_PoorCoverage = 151,
		ETC2_R8G8B8A8_SRGB_Block_PoorCoverage = 152,
		EAC_R11_UNorm_Block_PoorCoverage = 153,
		EAC_R11_SNorm_Block_PoorCoverage = 154,
		EAC_R11G11_UNorm_Block_PoorCoverage = 155,
		EAC_R11G11_SNorm_Block_PoorCoverage = 156,
		ASTC_4x4_UNorm_Block_PoorCoverage = 157,
		ASTC_4x4_SRGB_Block_PoorCoverage = 158,
		ASTC_5x4_UNorm_Block_PoorCoverage = 159,
		ASTC_5x4_SRGB_Block_PoorCoverage = 160,
		ASTC_5x5_UNorm_Block_PoorCoverage = 161,
		ASTC_5x5_SRGB_Block_PoorCoverage = 162,
		ASTC_6x5_UNorm_Block_PoorCoverage = 163,
		ASTC_6x5_SRGB_Block_PoorCoverage = 164,
		ASTC_6x6_UNorm_Block_PoorCoverage = 165,
		ASTC_6x6_SRGB_Block_PoorCoverage = 166,
		ASTC_8x5_UNorm_Block_PoorCoverage = 167,
		ASTC_8x5_SRGB_Block_PoorCoverage = 168,
		ASTC_8x6_UNorm_Block_PoorCoverage = 169,
		ASTC_8x6_SRGB_Block_PoorCoverage = 170,
		ASTC_8x8_UNorm_Block_PoorCoverage = 171,
		ASTC_8x8_SRGB_Block_PoorCoverage = 172,
		ASTC_10x5_UNorm_Block_PoorCoverage = 173,
		ASTC_10x5_SRGB_Block_PoorCoverage = 174,
		ASTC_10x6_UNorm_Block_PoorCoverage = 175,
		ASTC_10x6_SRGB_Block_PoorCoverage = 176,
		ASTC_10x8_UNorm_Block_PoorCoverage = 177,
		ASTC_10x8_SRGB_Block_PoorCoverage = 178,
		ASTC_10x10_UNorm_Block_PoorCoverage = 179,
		ASTC_10x10_SRGB_Block_PoorCoverage = 180,
		ASTC_12x10_UNorm_Block_PoorCoverage = 181,
		ASTC_12x10_SRGB_Block_PoorCoverage = 182,
		ASTC_12x12_UNorm_Block_PoorCoverage = 183,
		ASTC_12x12_SRGB_Block_PoorCoverage = 184
	};
#endif
}
