/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PROSPER_GL_UTIL_HPP__
#define __PROSPER_GL_UTIL_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_enums.hpp>

namespace prosper
{
	namespace util
	{
		DLLPROSPER_GL GLuint to_opengl_enum(prosper::Filter filter);
		DLLPROSPER_GL GLuint to_opengl_enum(prosper::SamplerAddressMode addressMode);
		DLLPROSPER_GL GLuint to_opengl_enum(prosper::CompareOp compareOp);
	};
};

#endif
