/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_fence.hpp"

using namespace prosper;

std::shared_ptr<IFence> GLFence::Create(IPrContext &context,GLsync fence)
{
	return std::shared_ptr<GLFence>{new GLFence{context,fence}};
}

GLFence::~GLFence()
{
	glDeleteSync(m_fence);
}
GLFence::GLFence(IPrContext &context,GLsync fence)
	: IFence{context},m_fence{fence}
{}
bool GLFence::IsSet() const
{
	GLint status;
	glGetSynciv(m_fence,GL_SYNC_STATUS,1,nullptr,&status);
	return status == GL_SIGNALED;
}
bool GLFence::Reset() const
{
	glDeleteSync(m_fence);
	glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
	return true;
}
