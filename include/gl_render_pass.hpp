// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PROSPER_GL_RENDER_PASS_HPP__
#define __PROSPER_GL_RENDER_PASS_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_render_pass.hpp>

namespace prosper {
	class DLLPROSPER_GL GLRenderPass : public prosper::IRenderPass {
	  public:
		static std::shared_ptr<IRenderPass> Create(IPrContext &context, const util::RenderPassCreateInfo &createInfo);

		virtual ~GLRenderPass() override;
	  private:
		GLRenderPass(IPrContext &context, const util::RenderPassCreateInfo &createInfo);
	};
};

#endif
