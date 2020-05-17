/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_framebuffer.hpp"
#include "gl_api.hpp"

using namespace prosper;

std::shared_ptr<IFramebuffer> GLFramebuffer::Create(
	IPrContext &context,const std::vector<std::shared_ptr<IImageView>> &attachments,
	uint32_t width,uint32_t height,uint32_t depth,uint32_t layers
)
{
	GLuint framebuffer;
	glGenFramebuffers(1,&framebuffer);
	return std::shared_ptr<GLFramebuffer>{new GLFramebuffer{context,attachments,width,height,depth,layers,framebuffer}};
}

GLFramebuffer::GLFramebuffer(
	IPrContext &context,const std::vector<std::shared_ptr<IImageView>> &attachments,
	uint32_t width,uint32_t height,uint32_t depth,uint32_t layers,GLuint framebuffer
)
	: IFramebuffer{context,attachments,width,height,depth,layers},m_framebuffer{framebuffer}
{}

GLFramebuffer::~GLFramebuffer()
{
	glDeleteFramebuffers(1,&m_framebuffer);
}
