/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_render_pass.hpp"

using namespace prosper;

std::shared_ptr<IRenderPass> GLRenderPass::Create(IPrContext &context,const prosper::util::RenderPassCreateInfo &createInfo)
{
	return std::shared_ptr<GLRenderPass>{new GLRenderPass{context,createInfo}};
}

GLRenderPass::~GLRenderPass() {}

GLRenderPass::GLRenderPass(IPrContext &context,const prosper::util::RenderPassCreateInfo &createInfo)
	: IRenderPass{context,createInfo}
{}
