/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PR_PROSPER_GL_IMAGE_HPP__
#define __PR_PROSPER_GL_IMAGE_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <image/prosper_image.hpp>

namespace prosper
{
	class GLContext;
	class GLFramebuffer;
	class DLLPROSPER_GL GLImage
		: public prosper::IImage
	{
	public:
		static std::shared_ptr<IImage> Create(IPrContext &context,const util::ImageCreateInfo &createInfo,const std::function<const uint8_t*(uint32_t layer,uint32_t mipmap,uint32_t &dataSize,uint32_t &rowSize)> &getImageData);
		static GLenum GetImageType(const util::ImageCreateInfo &createInfo);
		static bool IsLayered(const util::ImageCreateInfo &createInfo);

		virtual ~GLImage() override;
		virtual std::optional<util::SubresourceLayout> GetSubresourceLayout(uint32_t layerId=0,uint32_t mipMapIdx=0) override;
		virtual DeviceSize GetAlignment() const override;
		virtual bool Map(DeviceSize offset,DeviceSize size,void **outPtr=nullptr) override;
		virtual bool Unmap() override;
		virtual const void *GetInternalHandle() const override;
		uint64_t GetLayerSize(uint32_t w,uint32_t h) const;
		virtual bool WriteImageData(uint32_t x,uint32_t y,uint32_t w,uint32_t h,uint32_t layerIndex,uint32_t mipLevel,uint64_t size,const uint8_t *data) override;
		GLenum GetBufferBit() const;
		GLenum GetImageType() const;
		GLenum GetImageType(uint32_t layerIndex) const;
		GLuint GetGLImage() const {return m_image;}
		GLenum GetPixelDataFormat() const {return m_pixelDataFormat;}
		bool IsLayered() const;
		std::shared_ptr<GLFramebuffer> GetOrCreateFramebuffer(uint32_t baseLayerId,uint32_t layerCount,uint32_t baseMipmap,uint32_t mipmapCount);
	private:
		friend GLContext;
		GLImage(IPrContext &context,const util::ImageCreateInfo &createInfo,GLuint texture,GLenum pixelFormat);
		virtual bool DoSetMemoryBuffer(IBuffer &buffer) override;
		void InitializeSubresourceLayouts();
		GLuint m_image = GL_INVALID_VALUE;
		GLenum m_pixelDataFormat;

		std::vector<std::shared_ptr<GLFramebuffer>> m_framebuffers;
		std::vector<std::vector<prosper::util::SubresourceLayout>> m_subresourceLayouts {};
	};
};

#endif
