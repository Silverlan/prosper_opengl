/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_framebuffer.hpp"
#include "gl_api.hpp"
#include "image/gl_image.hpp"
#include "image/gl_image_view.hpp"

using namespace prosper;

std::shared_ptr<IFramebuffer> GLFramebuffer::Create(
	IPrContext &context,const std::vector<std::shared_ptr<IImageView>> &attachments,
	uint32_t width,uint32_t height,uint32_t depth,uint32_t layers
)
{
	GLint drawFboId = 0;
	GLint readFboId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&drawFboId);
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&readFboId);

	GLuint framebuffer;
	glCreateFramebuffers(1,&framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
	uint32_t attId = 0;
	for(auto &att : attachments)
	{
		auto mipmapLevel = att->GetBaseMipmapLevel();
		auto aspectMask = att->GetAspectMask();
		auto baseLayer = att->GetBaseLayer();
		auto numLayers = att->GetLayerCount();
		auto texId = static_cast<GLImage&>(att->GetImage()).GetGLImage();
		GLenum attachment;
		switch(aspectMask)
		{
		case prosper::ImageAspectFlags::DepthBit:
			attachment = GL_DEPTH_ATTACHMENT;
			break;
		case prosper::ImageAspectFlags::ColorBit:
			attachment = GL_COLOR_ATTACHMENT0 +attId;
			break;
		default:
			continue; // Should be unreachable
		}
		if(numLayers == 1 && baseLayer == 0)
			glFramebufferTexture2D(GL_FRAMEBUFFER,attachment,GL_TEXTURE_2D,texId,mipmapLevel);
		else
		{
			for(auto i=baseLayer;i<(baseLayer +numLayers);++i)
				glFramebufferTexture3D(GL_FRAMEBUFFER,attachment,GL_TEXTURE_3D,texId,mipmapLevel,i);
		}
		++attId;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,drawFboId);
	glBindFramebuffer(GL_READ_FRAMEBUFFER,readFboId);

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
	if(m_framebuffer != 0)
		glDeleteFramebuffers(1,&m_framebuffer);
}
