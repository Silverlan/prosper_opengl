// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PROSPER_GL_UTIL_HPP__
#define __PROSPER_GL_UTIL_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_enums.hpp>

typedef unsigned int GLenum;
namespace prosper {
	namespace util {
		DLLPROSPER_GL GLenum to_opengl_enum(prosper::Filter filter);
		DLLPROSPER_GL GLenum to_opengl_enum(prosper::SamplerAddressMode addressMode);
		DLLPROSPER_GL GLenum to_opengl_enum(prosper::CompareOp compareOp);
		DLLPROSPER_GL GLenum to_opengl_enum(prosper::PrimitiveTopology primitiveTopology);
		DLLPROSPER_GL GLenum to_opengl_enum(prosper::BlendOp blendOp);
		DLLPROSPER_GL GLenum to_opengl_enum(prosper::BlendFactor blendFactor);
		DLLPROSPER_GL GLenum to_opengl_enum(prosper::IndexType indexType);
		DLLPROSPER_GL GLenum to_opengl_image_format_type(prosper::Format format, GLboolean &outNormalized);
		DLLPROSPER_GL GLenum to_opengl_image_format(prosper::Format format, GLenum *optOutPixelDataFormat = nullptr);
	};
};

#endif
