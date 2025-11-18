// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "opengl_api.hpp"

export module pragma.prosper.opengl:image.image;

export import pragma.prosper;

export namespace prosper {
	class GLContext;
	class GLFramebuffer;
	class GLWindow;
	class PR_EXPORT GLImage : public prosper::IImage {
	  public:
		static std::shared_ptr<IImage> Create(IPrContext &context, const util::ImageCreateInfo &createInfo, const std::function<const uint8_t *(uint32_t layer, uint32_t mipmap, uint32_t &dataSize, uint32_t &rowSize)> &getImageData);
		static GLenum GetImageType(const util::ImageCreateInfo &createInfo);
		static bool IsLayered(const util::ImageCreateInfo &createInfo);

		virtual ~GLImage() override;
		virtual std::optional<util::SubresourceLayout> GetSubresourceLayout(uint32_t layerId = 0, uint32_t mipMapIdx = 0) override;
		virtual DeviceSize GetAlignment() const override;
		virtual bool Map(DeviceSize offset, DeviceSize size, void **outPtr = nullptr) override;
		virtual bool Unmap() override;
		virtual const void *GetInternalHandle() const override;
		virtual std::optional<size_t> GetStorageSize() const override;
		uint64_t GetLayerSize(uint32_t w, uint32_t h) const;
		virtual bool WriteImageData(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t layerIndex, uint32_t mipLevel, uint64_t size, const uint8_t *data) override;
		GLenum GetBufferBit() const;
		GLenum GetImageType() const;
		GLenum GetImageType(uint32_t layerIndex) const;
		GLuint GetGLImage() const { return m_image; }
		GLenum GetPixelDataFormat() const { return m_pixelDataFormat; }
		bool IsLayered() const;
		std::shared_ptr<GLFramebuffer> GetOrCreateFramebuffer(uint32_t baseLayerId, uint32_t layerCount, uint32_t baseMipmap, uint32_t mipmapCount);
	  private:
		friend GLContext;
		friend GLWindow;
		GLImage(IPrContext &context, const util::ImageCreateInfo &createInfo, GLuint texture, GLenum pixelFormat);
		virtual bool DoSetMemoryBuffer(IBuffer &buffer) override;
		void InitializeSubresourceLayouts();
		GLuint m_image = GL_INVALID_VALUE;
		GLenum m_pixelDataFormat;

		std::vector<std::shared_ptr<GLFramebuffer>> m_framebuffers;
		std::vector<std::vector<prosper::util::SubresourceLayout>> m_subresourceLayouts {};
	};
};
