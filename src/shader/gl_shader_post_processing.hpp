// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __GL_SHADER_POST_PROCESSING_HPP__
#define __GL_SHADER_POST_PROCESSING_HPP__

#include <vector>
#include <string>

namespace prosper::util
{
	bool convert_glsl_set_bindings_to_opengl_binding_points(std::vector<std::string> &inOutGlslCodePerStage,std::string &outErrMsg);
};

#endif
