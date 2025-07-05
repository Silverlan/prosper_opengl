// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "gl_render_pass.hpp"

using namespace prosper;

std::shared_ptr<IRenderPass> GLRenderPass::Create(IPrContext &context, const prosper::util::RenderPassCreateInfo &createInfo) { return std::shared_ptr<GLRenderPass> {new GLRenderPass {context, createInfo}}; }

GLRenderPass::~GLRenderPass() {}

GLRenderPass::GLRenderPass(IPrContext &context, const prosper::util::RenderPassCreateInfo &createInfo) : IRenderPass {context, createInfo} {}
