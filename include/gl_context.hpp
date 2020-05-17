/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PROSPER_GL_CONTEXT_HPP__
#define __PROSPER_GL_CONTEXT_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_context.hpp>
#include <array>
#include <queue>
#include <memory>
#include <optional>

namespace prosper
{
	class GLBuffer;
	class DLLPROSPER_GL GLContext
		: public IPrContext
	{
	public:
		static std::shared_ptr<GLContext> Create(const std::string &appName,bool bEnableValidation);
		virtual bool IsImageFormatSupported(
			prosper::Format format,prosper::ImageUsageFlags usageFlags,prosper::ImageType type=prosper::ImageType::e2D,
			prosper::ImageTiling tiling=prosper::ImageTiling::Optimal
		) const override;
		virtual ~GLContext() override;

		virtual void ReloadWindow() override;

		virtual bool IsPresentationModeSupported(prosper::PresentModeKHR presentMode) const override;
		virtual Vendor GetPhysicalDeviceVendor() const override;
		virtual MemoryRequirements GetMemoryRequirements(IImage &img) override;
		virtual DeviceSize GetBufferAlignment(BufferUsageFlags usageFlags) override;

		virtual bool SavePipelineCache() override;

		virtual std::shared_ptr<prosper::IPrimaryCommandBuffer> AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType queueFamilyType,uint32_t &universalQueueFamilyIndex) override;
		virtual std::shared_ptr<prosper::ISecondaryCommandBuffer> AllocateSecondaryLevelCommandBuffer(prosper::QueueFamilyType queueFamilyType,uint32_t &universalQueueFamilyIndex) override;
		virtual void SubmitCommandBuffer(prosper::ICommandBuffer &cmd,prosper::QueueFamilyType queueFamilyType,bool shouldBlock=false,prosper::IFence *fence=nullptr) override;

		virtual Result WaitForFence(const IFence &fence,uint64_t timeout=std::numeric_limits<uint64_t>::max()) const override;
		virtual Result WaitForFences(const std::vector<IFence*> &fences,bool waitAll=true,uint64_t timeout=std::numeric_limits<uint64_t>::max()) const override;
		virtual void DrawFrame(const std::function<void(const std::shared_ptr<prosper::IPrimaryCommandBuffer>&,uint32_t)> &drawFrame) override;
		virtual bool Submit(ICommandBuffer &cmdBuf,bool shouldBlock=false,IFence *optFence=nullptr) override;

		virtual std::shared_ptr<IBuffer> CreateBuffer(const util::BufferCreateInfo &createInfo,const void *data=nullptr) override;
		virtual std::shared_ptr<IEvent> CreateEvent() override;
		virtual std::shared_ptr<IFence> CreateFence(bool createSignalled=false) override;
		virtual std::shared_ptr<ISampler> CreateSampler(const util::SamplerCreateInfo &createInfo) override;
		virtual std::shared_ptr<IImage> CreateImage(const util::ImageCreateInfo &createInfo,const uint8_t *data=nullptr) override;
		virtual std::shared_ptr<IRenderPass> CreateRenderPass(const util::RenderPassCreateInfo &renderPassInfo) override;
		virtual std::shared_ptr<IDescriptorSetGroup> CreateDescriptorSetGroup(DescriptorSetCreateInfo &descSetInfo) override;
		virtual std::shared_ptr<IFramebuffer> CreateFramebuffer(uint32_t width,uint32_t height,uint32_t layers,const std::vector<prosper::IImageView*> &attachments) override;

		bool CheckResult();
		GLBuffer &GetPushConstantBuffer() const;
	protected:
		GLContext(const std::string &appName,bool bEnableValidation=false);
		virtual std::shared_ptr<IImageView> DoCreateImageView(
			const util::ImageViewCreateInfo &createInfo,IImage &img,Format format,ImageViewType imgViewType,prosper::ImageAspectFlags aspectMask,uint32_t numLayers
		) override;
		virtual void DoKeepResourceAliveUntilPresentationComplete(const std::shared_ptr<void> &resource) override;
		virtual void DoWaitIdle() override;
		virtual void DoFlushSetupCommandBuffer() override;
		virtual void ReloadSwapchain() override;

		virtual void InitAPI(const CreateInfo &createInfo) override;
		void InitPushConstantBuffer();
	private:
		std::shared_ptr<IBuffer> m_pushConstantBuffer = nullptr;
	};
};

#endif
