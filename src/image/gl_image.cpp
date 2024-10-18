/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "image/gl_image.hpp"
#include "image/gl_image_view.hpp"
#include "gl_framebuffer.hpp"
#include "gl_context.hpp"
#include "gl_api.hpp"
#include "gl_util.hpp"
#include <prosper_util.hpp>

using namespace prosper;

#if 0
#include <gli/gli.hpp>
#include <iostream>
#include <prosper_util.hpp>
static void test_cubemap()
{
	auto ddsTex = gli::load_dds("E:/projects/pragma/build_winx64/output/addons/converted/materials/skybox/sky_day01_08.dds");
	uint32_t w = ddsTex.extent().x;
	uint32_t h = ddsTex.extent().y;
	GLuint tex;
	auto type = GL_TEXTURE_CUBE_MAP;
	glCreateTextures(type,1,&tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(type,tex);
	static_cast<GLContext&>(context).CheckResult();
	// TODO: Miplevels
	glTextureStorage2D(tex,1,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,w,h);
	static_cast<GLContext&>(context).CheckResult();

	glBindTexture(type,tex);

	GLint size;
	glGetTextureLevelParameteriv(tex,0 /* mipLevel */,GL_TEXTURE_COMPRESSED_IMAGE_SIZE,&size);
	static_cast<GLContext&>(context).CheckResult();

	auto *data = ddsTex.data(0,0,0);
	auto size2 = ddsTex.size(0);
	std::cout<<"Size: "<<size<<","<<size2<<std::endl;
	glCompressedTexSubImage2D(
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,0 /* mipLevel */,0,0,w,h,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,size2,data
	);
	static_cast<GLContext&>(context).CheckResult();
}
#endif

std::shared_ptr<IImage> GLImage::Create(IPrContext &context,const prosper::util::ImageCreateInfo &createInfo,const std::function<const uint8_t*(uint32_t layer,uint32_t mipmap,uint32_t &dataSize,uint32_t &rowSize)> &getImageData)
{
	auto isCubemap = umath::is_flag_set(createInfo.flags,util::ImageCreateInfo::Flags::Cubemap);
	auto type = GetImageType(createInfo);
	GLuint tex;
	glCreateTextures(type,1,&tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(type,tex);

	uint32_t mipLevels = 1;
	if(umath::is_flag_set(createInfo.flags,prosper::util::ImageCreateInfo::Flags::FullMipmapChain))
		mipLevels = prosper::util::calculate_mipmap_count(createInfo.width,createInfo.height);
	GLenum pixelFormat;
	auto format = prosper::util::to_opengl_image_format(createInfo.format,&pixelFormat);
	if(IsLayered(createInfo) == false)
		glTextureStorage2D(tex,mipLevels,format,createInfo.width,createInfo.height);
	else
		glTextureStorage3D(tex,mipLevels,format,createInfo.width,createInfo.height,createInfo.layers);

	auto img = std::shared_ptr<GLImage>{new GLImage{context,createInfo,tex,pixelFormat}};
	if(static_cast<GLContext&>(context).CheckResult() == false)
		return nullptr;
	if(getImageData)
	{
		auto numMipmaps = umath::is_flag_set(createInfo.flags,util::ImageCreateInfo::Flags::FullMipmapChain) ? util::calculate_mipmap_count(createInfo.width,createInfo.height) : 1u;
		for(auto iLayer=decltype(createInfo.layers){0u};iLayer<createInfo.layers;++iLayer)
		{
			for(auto iMipmap=decltype(numMipmaps){0u};iMipmap<numMipmaps;++iMipmap)
			{
				auto w = img->GetWidth(iMipmap);
				auto h = img->GetHeight(iMipmap);
				uint32_t rowSize = img->GetLayerSize(w,1);
				uint32_t dataSize = img->GetLayerSize(w,h);
				auto *mipmapData = getImageData(iLayer,iMipmap,dataSize,rowSize);
				if(mipmapData == nullptr)
					continue;
				if(img->WriteImageData(0,0,w,h,iLayer,iMipmap,dataSize,mipmapData) == false)
					return nullptr;
			}
		}
	}
	static_cast<GLContext&>(context).CheckResult();
	img->InitializeSubresourceLayouts();
	return img;
}
uint64_t GLImage::GetLayerSize(uint32_t w,uint32_t h) const
{
	return w *h *(prosper::util::is_compressed_format(GetFormat()) ? prosper::util::get_block_size(GetFormat()) : prosper::util::get_byte_size(GetFormat()));
}
bool GLImage::WriteImageData(uint32_t x,uint32_t y,uint32_t w,uint32_t h,uint32_t layerIndex,uint32_t mipLevel,uint64_t size,const uint8_t *data)
{
	auto type = GetImageType(layerIndex);
	auto is3DType = (type == GL_TEXTURE_2D_ARRAY || type == GL_TEXTURE_3D);
	auto isCubemap = IsCubemap();
	glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : type,GetGLImage());
	if(prosper::util::is_compressed_format(GetFormat()))
	{
		if(w != util::calculate_mipmap_size(GetWidth(),mipLevel) || h != util::calculate_mipmap_size(GetHeight(),mipLevel))
			return false;
		auto format = prosper::util::to_opengl_image_format(GetFormat());
		if(is3DType == false)
		{
			glCompressedTexSubImage2D(
				type,mipLevel,x,y,w,h,format,size,data
			);
		}
		else
		{
			glCompressedTexSubImage3D(
				type,mipLevel,x,y,layerIndex,w,h,1,format,size,data
			);
		}
		return static_cast<GLContext&>(GetContext()).CheckResult();
	}
	GLboolean normalized;
	auto imgFormatType = util::to_opengl_image_format_type(GetFormat(),normalized);
	if(is3DType == false)
	{
		glTexSubImage2D(
			type,mipLevel,x,y,w,h,GetPixelDataFormat(),imgFormatType,data
		);
	}
	else
	{
		glTexSubImage3D(
			type,mipLevel,x,y,layerIndex,w,h,1,GetPixelDataFormat(),imgFormatType,data
		);
	}
	return static_cast<GLContext&>(GetContext()).CheckResult();
}
bool GLImage::IsLayered() const {return IsLayered(GetCreateInfo());}
bool GLImage::IsLayered(const prosper::util::ImageCreateInfo &createInfo) {return (createInfo.layers > 1 && umath::is_flag_set(createInfo.flags,util::ImageCreateInfo::Flags::Cubemap) == false);}
GLenum GLImage::GetImageType(const prosper::util::ImageCreateInfo &createInfo)
{
	GLenum type = GL_TEXTURE_2D;
	if(umath::is_flag_set(createInfo.flags,prosper::util::ImageCreateInfo::Flags::Cubemap))
		type = GL_TEXTURE_CUBE_MAP;
	else
	{
		switch(createInfo.type)
		{
		case prosper::ImageType::e1D:
			type = IsLayered(createInfo) ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
			break;
		case prosper::ImageType::e2D:
			type = IsLayered(createInfo) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
			break;
		case prosper::ImageType::e3D:
			type = GL_TEXTURE_3D;
			break;
		}
	}
	return type;
}

/////////////

GLImage::GLImage(IPrContext &context,const prosper::util::ImageCreateInfo &createInfo,GLuint texture,GLenum pixelFormat)
	: IImage{context,createInfo},m_image{texture},m_pixelDataFormat{pixelFormat}
{}
GLImage::~GLImage()
{
	if(m_image != 0)
		glDeleteTextures(1,&m_image);
}
GLenum GLImage::GetBufferBit() const
{
	return prosper::util::is_depth_format(GetFormat()) ? GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
}
GLenum GLImage::GetImageType() const
{
	return GetImageType(GetCreateInfo());
}
GLenum GLImage::GetImageType(uint32_t layerIndex) const
{
	auto type = GetImageType();
	if(type != GL_TEXTURE_CUBE_MAP)
		return type;
	switch(layerIndex)
	{
	case 0:
		return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	case 1:
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
	case 2:
		return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
	case 3:
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
	case 4:
		return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
	case 5:
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
	}
	return GL_INVALID_VALUE;
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
void GLImage::InitializeSubresourceLayouts()
{
	auto numLayers = GetLayerCount();
	auto numMipmaps = GetMipmapCount();

	auto format = GetFormat();
	auto szPerPixel = prosper::util::is_compressed_format(format) ? prosper::util::get_block_size(format) : prosper::util::get_byte_size(format);

	m_subresourceLayouts.resize(numLayers,std::vector<prosper::util::SubresourceLayout>{numMipmaps});
	DeviceSize offset = 0;
	for(auto iLayer=decltype(numLayers){0u};iLayer<numLayers;++iLayer)
	{
		for(auto iMipmap=decltype(numMipmaps){0u};iMipmap<numMipmaps;++iMipmap)
		{
			auto w = GetWidth(iMipmap);
			auto h = GetHeight(iMipmap);
			auto size = w *h *szPerPixel;
			prosper::util::SubresourceLayout layout {};
			layout.size = size;
			layout.row_pitch = w *szPerPixel;
			layout.depth_pitch = w *h *szPerPixel;
			layout.array_pitch = w *h *numMipmaps *szPerPixel;
			layout.offset = offset;
			m_subresourceLayouts.at(iLayer).at(iMipmap) = layout;

			offset += size;
		}
	}
}
std::optional<prosper::util::SubresourceLayout> GLImage::GetSubresourceLayout(uint32_t layerId,uint32_t mipMapIdx)
{
	if(layerId >= m_subresourceLayouts.size())
		return {};
	auto &layerLayouts = m_subresourceLayouts.at(layerId);
	if(mipMapIdx >= layerLayouts.size())
		return {};
	return layerLayouts.at(mipMapIdx);
}
DeviceSize GLImage::GetAlignment() const
{
	return 0; // TODO
}
bool GLImage::Map(DeviceSize offset,DeviceSize size,void **outPtr)
{
	return false; // TODO
}

bool GLImage::Unmap()
{
	return false; // TODO
}

const void *GLImage::GetInternalHandle() const { return reinterpret_cast<void *>(m_image); }

std::optional<size_t> prosper::GLImage::GetStorageSize() const  
{
		return {4};
}

bool GLImage::DoSetMemoryBuffer(IBuffer &buffer)
{
	return false; // TODO
}
