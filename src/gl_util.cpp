/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_util.hpp"
#include "gl_api.hpp"

using namespace prosper;

GLuint prosper::util::to_opengl_enum(prosper::Filter filter)
{
	auto glFilter = GL_LINEAR;
	switch(filter)
	{
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
	switch(addressMode)
	{
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
	switch(compareOp)
	{
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
