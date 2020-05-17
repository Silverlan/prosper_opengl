/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PROSPER_GL_RENDER_PASS_HPP__
#define __PROSPER_GL_RENDER_PASS_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_render_pass.hpp>

namespace prosper
{
	class DLLPROSPER_GL GLRenderPass
		: public prosper::IRenderPass
	{
	public:
		static std::shared_ptr<IRenderPass> Create(IPrContext &context,const util::RenderPassCreateInfo &createInfo);

		virtual ~GLRenderPass() override;
	private:
		GLRenderPass(IPrContext &context,const util::RenderPassCreateInfo &createInfo);
	};
};

#endif
