/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_context.hpp"
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
	GLuint framebuffer;
	glCreateFramebuffers(1,&framebuffer);
	uint32_t attId = 0;
	std::vector<GLenum> bufferTargets {};
	bufferTargets.reserve(attachments.size());
	for(auto &att : attachments)
	{
		auto mipmapLevel = att->GetBaseMipmapLevel();
		auto aspectMask = att->GetAspectMask();
		auto baseLayer = att->GetBaseLayer();
		auto numLayers = att->GetLayerCount();
		auto &img = static_cast<GLImage&>(att->GetImage());
		auto type = img.GetImageType();
		auto texId = img.GetGLImage();
		GLenum attachment;
		switch(aspectMask)
		{
		case prosper::ImageAspectFlags::DepthBit:
			attachment = GL_DEPTH_ATTACHMENT;
			bufferTargets.push_back(GL_NONE);
			break;
		case prosper::ImageAspectFlags::ColorBit:
			attachment = GL_COLOR_ATTACHMENT0 +attId;
			bufferTargets.push_back(attachment);
			break;
		default:
			bufferTargets.push_back(GL_NONE);
			continue; // Should be unreachable
		}
		if(baseLayer == 0)
			glNamedFramebufferTexture(framebuffer,attachment,texId,mipmapLevel);
		else
			glNamedFramebufferTextureLayer(framebuffer,attachment,texId,mipmapLevel,baseLayer);
		++attId;
	}
	glNamedFramebufferDrawBuffers(framebuffer,bufferTargets.size(),bufferTargets.data());

	static_cast<GLContext&>(context).CheckResult();
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
void GLFramebuffer::UpateSize(uint32_t w,uint32_t h)
{
	m_width = w;
	m_height = h;
}
const void *GLFramebuffer::GetInternalHandle() const {return reinterpret_cast<void*>(m_framebuffer);}
