// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.prosper.opengl:shader.post_processing;

export import std;

namespace prosper::util
{
	bool convert_glsl_set_bindings_to_opengl_binding_points(std::vector<std::string> &inOutGlslCodePerStage,std::string &outErrMsg);
};
