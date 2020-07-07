/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "image/gl_image.hpp"
#include "image/gl_image_view.hpp"
#include "gl_framebuffer.hpp"
#include "gl_context.hpp"
#include "gl_api.hpp"
#include "gl_util.hpp"

using namespace prosper;

#pragma optimize("",off)
std::shared_ptr<IImage> GLImage::Create(IPrContext &context,const prosper::util::ImageCreateInfo &createInfo)
{
	GLenum type = GL_TEXTURE_2D;
	switch(createInfo.type)
	{
	case prosper::ImageType::e1D:
		type = GL_TEXTURE_1D;
		break;
	case prosper::ImageType::e2D:
		type = GL_TEXTURE_2D;
		break;
	case prosper::ImageType::e3D:
		type = GL_TEXTURE_3D;
		break;
	}
	GLuint tex;
	glCreateTextures(type,1,&tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(type,tex);

	uint32_t mipLevels = 1;
	if(umath::is_flag_set(createInfo.flags,prosper::util::ImageCreateInfo::Flags::FullMipmapChain))
		mipLevels = prosper::util::calculate_mipmap_count(createInfo.width,createInfo.height);
	auto format = prosper::util::to_opengl_image_format(createInfo.format);
	glTextureStorage2D(tex,mipLevels,format,createInfo.width,createInfo.height);
	static_cast<GLContext&>(context).CheckResult();
	return std::shared_ptr<GLImage>{new GLImage{context,createInfo,tex}};
}
GLImage::GLImage(IPrContext &context,const prosper::util::ImageCreateInfo &createInfo,GLuint texture)
	: IImage{context,createInfo},m_image{texture}
{}
GLImage::~GLImage()
{
	if(m_image != 0)
		glDeleteTextures(1,&m_image);
}
std::shared_ptr<GLFramebuffer> GLImage::GetOrCreateFramebuffer(uint32_t baseLayerId,uint32_t layerCount,uint32_t baseMipmap,uint32_t mipmapCount)
{
	if(m_image == 0)
		return std::static_pointer_cast<GLFramebuffer>(static_cast<GLContext&>(GetContext()).GetSwapchainFramebuffer(0)->shared_from_this());
	auto it = std::find_if(m_framebuffers.begin(),m_framebuffers.end(),[baseLayerId,layerCount,baseMipmap,mipmapCount](const std::shared_ptr<GLFramebuffer> &framebuffer) {
		if(layerCount != framebuffer->GetLayerCount() || framebuffer->GetAttachmentCount() != 1)
			return false;
		auto &imgView = *framebuffer->GetAttachment(0);
		return imgView.GetBaseLayer() == baseLayerId && imgView.GetBaseMipmapLevel() == baseMipmap && imgView.GetMipmapCount() == mipmapCount;
	});
	if(it != m_framebuffers.end())
		return *it;

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.baseLayer = baseLayerId;
	imgViewCreateInfo.baseMipmap = baseMipmap;
	imgViewCreateInfo.mipmapLevels = mipmapCount;
	// OpenGL doesn't have image views, so for the OpenGL implementation
	// they just serve as a meta information struct. They also don't use up
	// any OpenGL resources, so we can just allocate however many we want/need.
	auto imgView = GetContext().CreateImageView(imgViewCreateInfo,*this);
	auto framebuffer = GetContext().CreateFramebuffer(GetWidth(),GetHeight(),layerCount,{imgView.get()});
	static_cast<GLContext&>(GetContext()).CheckFramebufferStatus(*framebuffer);
	m_framebuffers.push_back(std::static_pointer_cast<GLFramebuffer>(framebuffer));
	return m_framebuffers.back();
}
std::optional<prosper::util::SubresourceLayout> GLImage::GetSubresourceLayout(uint32_t layerId,uint32_t mipMapIdx)
{
	prosper::util::SubresourceLayout layout {};
	// TODO
#if 0
	struct DLLPROSPER SubresourceLayout
	{
		DeviceSize offset;
		DeviceSize size;
		DeviceSize row_pitch;
		DeviceSize array_pitch;
		DeviceSize depth_pitch;
	};
#endif
	return layout; // TODO
}
DeviceSize GLImage::GetAlignment() const
{
	return 0; // TODO
}
bool GLImage::Map(DeviceSize offset,DeviceSize size,void **outPtr)
{
	return false; // TODO
}

bool GLImage::DoSetMemoryBuffer(IBuffer &buffer)
{
	return false; // TODO
}
#pragma optimize("",on)
