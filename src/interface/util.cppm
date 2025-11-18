// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "opengl_api.hpp"

export module pragma.prosper.opengl:util;

export import pragma.prosper;

typedef unsigned int GLenum;
export namespace prosper {
	namespace util {
		PR_EXPORT GLenum to_opengl_enum(prosper::Filter filter);
		PR_EXPORT GLenum to_opengl_enum(prosper::SamplerAddressMode addressMode);
		PR_EXPORT GLenum to_opengl_enum(prosper::CompareOp compareOp);
		PR_EXPORT GLenum to_opengl_enum(prosper::PrimitiveTopology primitiveTopology);
		PR_EXPORT GLenum to_opengl_enum(prosper::BlendOp blendOp);
		PR_EXPORT GLenum to_opengl_enum(prosper::BlendFactor blendFactor);
		PR_EXPORT GLenum to_opengl_enum(prosper::IndexType indexType);
		PR_EXPORT GLenum to_opengl_image_format_type(prosper::Format format, GLboolean &outNormalized);
		PR_EXPORT GLenum to_opengl_image_format(prosper::Format format, GLenum *optOutPixelDataFormat = nullptr);
	};
};
