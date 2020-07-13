/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __GL_SHADER_POST_PROCESSING_HPP__
#define __GL_SHADER_POST_PROCESSING_HPP__

#include <vector>
#include <string>

namespace prosper::util
{
	bool convert_glsl_set_bindings_to_opengl_binding_points(std::vector<std::string> &inOutGlslCodePerStage,std::string &outErrMsg);
};

#endif
