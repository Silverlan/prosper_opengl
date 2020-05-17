/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "prosper_opengl_definitions.hpp"
#include <gl_context.hpp>

extern "C"
{
	DLLPROSPER_GL bool initialize_render_api(const std::string &engineName,bool enableValidation,std::shared_ptr<prosper::IPrContext> &outContext,std::string &errMsg)
	{
		outContext = prosper::GLContext::Create(engineName,enableValidation);
		return true;
	}
};
