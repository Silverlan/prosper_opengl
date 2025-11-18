// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "opengl_api.hpp"

module pragma.prosper.opengl;

import :fence;

using namespace prosper;

std::shared_ptr<IFence> GLFence::Create(IPrContext &context) { return std::shared_ptr<GLFence> {new GLFence {context}}; }

GLFence::~GLFence() { Clear(); }
void GLFence::Clear() const
{
	if(m_fence == nullptr)
		return;
	glDeleteSync(m_fence);
	m_fence = nullptr;
	static_cast<GLContext &>(GetContext()).CheckResult();
}
GLFence::GLFence(IPrContext &context) : IFence {context} {}
bool GLFence::IsSet() const
{
	if(m_fence == nullptr)
		return true;
	GLint status;
	glGetSynciv(m_fence, GL_SYNC_STATUS, 1, nullptr, &status);
	return status == GL_SIGNALED;
}
bool GLFence::Reset() const
{
	Clear();
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	return static_cast<GLContext &>(GetContext()).CheckResult();
}
void GLFence::Wait()
{
	glClientWaitSync(m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, std::numeric_limits<GLuint64>::max());
	static_cast<GLContext &>(GetContext()).CheckResult();
}
