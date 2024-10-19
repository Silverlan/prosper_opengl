/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PROSPER_GL_CONTEXT_HPP__
#define __PROSPER_GL_CONTEXT_HPP__

#include <shader/prosper_shader.hpp>
#include <prosper_opengl_definitions.hpp>
#include <prosper_context.hpp>
#include <array>
#include <queue>
#include <memory>
#include <optional>

class GLShaderProgram;
namespace prosper {
	class ShaderBlit;
	class ShaderFlipImage;
	class GLBuffer;
	class BasePipelineCreateInfo;

	class DLLPROSPER_GL GLContext : public IPrContext {
	  public:
		static std::shared_ptr<GLContext> Create(const std::string &appName, bool bEnableValidation);
		bool CheckFramebufferStatus(IFramebuffer &fb) const;
		virtual bool IsImageFormatSupported(prosper::Format format, prosper::ImageUsageFlags usageFlags, prosper::ImageType type = prosper::ImageType::e2D, prosper::ImageTiling tiling = prosper::ImageTiling::Optimal) const override;
		virtual uint32_t GetUniversalQueueFamilyIndex() const override;
		virtual util::Limits GetPhysicalDeviceLimits() const override;
		virtual std::optional<util::PhysicalDeviceImageFormatProperties> GetPhysicalDeviceImageFormatProperties(const ImageFormatPropertiesQuery &query) override;
		virtual ~GLContext() override;
		virtual std::string GetAPIIdentifier() const override { return "OpenGL"; }
		virtual std::string GetAPIAbbreviation() const override { return "GL"; }
		virtual bool WaitForCurrentSwapchainCommandBuffer(std::string &outErrMsg) override;
		virtual std::shared_ptr<Window> CreateWindow(const WindowSettings &windowCreationInfo) override;

		virtual prosper::FeatureSupport AreFormatFeaturesSupported(Format format, FormatFeatureFlags featureFlags, std::optional<ImageTiling> tiling) const override;
		virtual void BakeShaderPipeline(prosper::PipelineID pipelineId, prosper::PipelineBindPoint pipelineType) override;

		virtual void ReloadWindow() override;
		prosper::IFramebuffer *GetSwapchainFramebuffer(uint32_t idx);

		//bool BuildShader(prosper::ShaderStage stage,const std::string &glslCode);
		virtual bool IsPresentationModeSupported(prosper::PresentModeKHR presentMode) const override;
		virtual Vendor GetPhysicalDeviceVendor() const override;
		virtual MemoryRequirements GetMemoryRequirements(IImage &img) override;
		virtual uint64_t ClampDeviceMemorySize(uint64_t size, float percentageOfGPUMemory, MemoryFeatureFlags featureFlags) const override;
		virtual DeviceSize CalcBufferAlignment(BufferUsageFlags usageFlags) override;

		virtual bool ShouldFlipTexturesOnLoad() const override { return false; }
		virtual uint32_t GetReservedDescriptorResourceCount(DescriptorResourceType resType) const override
		{
			if(resType == DescriptorResourceType::UniformBufferObject)
				return 1; // Index 0 is reserved for push constant buffer
			return 0;
		}

		virtual void GetGLSLDefinitions(glsl::Definitions &outDef) const override;

		virtual bool SavePipelineCache() override;

		virtual std::shared_ptr<prosper::IPrimaryCommandBuffer> AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType queueFamilyType, uint32_t &universalQueueFamilyIndex) override;
		virtual std::shared_ptr<prosper::ISecondaryCommandBuffer> AllocateSecondaryLevelCommandBuffer(prosper::QueueFamilyType queueFamilyType, uint32_t &universalQueueFamilyIndex) override;
		virtual std::shared_ptr<prosper::ICommandBufferPool> CreateCommandBufferPool(prosper::QueueFamilyType queueFamilyType) override;
		virtual void SubmitCommandBuffer(prosper::ICommandBuffer &cmd, prosper::QueueFamilyType queueFamilyType, bool shouldBlock = false, prosper::IFence *fence = nullptr) override;

		virtual std::shared_ptr<IBuffer> CreateBuffer(const util::BufferCreateInfo &createInfo, const void *data = nullptr) override;
		virtual std::shared_ptr<IDynamicResizableBuffer> CreateDynamicResizableBuffer(util::BufferCreateInfo createInfo, uint64_t maxTotalSize, float clampSizeToAvailableGPUMemoryPercentage = 1.f, const void *data = nullptr) override;
		virtual std::shared_ptr<IImage> CreateImage(const util::ImageCreateInfo &createInfo, const std::function<const uint8_t *(uint32_t layer, uint32_t mipmap, uint32_t &dataSize, uint32_t &rowSize)> &getImageData = nullptr) override;

		virtual void Flush() override;
		virtual Result WaitForFence(const IFence &fence, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const override;
		virtual Result WaitForFences(const std::vector<IFence *> &fences, bool waitAll = true, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const override;

		virtual std::unique_ptr<ShaderModule> CreateShaderModuleFromStageData(const std::shared_ptr<ShaderStageProgram> &shaderStageProgram, prosper::ShaderStage stage, const std::string &entrypointName = "main") override;
		virtual std::shared_ptr<ShaderStageProgram> CompileShader(prosper::ShaderStage stage, const std::string &shaderPath, std::string &outInfoLog, std::string &outDebugInfoLog, bool reload = false, const std::string &prefixCode = {},
		  const std::unordered_map<std::string, std::string> &definitions = {}) override;
		virtual bool GetParsedShaderSourceCode(prosper::Shader &shader, std::vector<std::string> &outGlslCodePerStage, std::vector<prosper::ShaderStage> &outGlslCodeStages, std::string &outInfoLog, std::string &outDebugInfoLog, prosper::ShaderStage &outErrStage) const override;
		bool GetParsedShaderSourceCode(prosper::Shader &shader, std::vector<std::string> &outGlslCodePerStage, std::vector<prosper::ShaderStage> &outGlslCodeStages, std::string &outInfoLog, std::string &outDebugInfoLog, prosper::ShaderStage &outErrStage, const std::string &prefixCode,
		  const std::unordered_map<std::string, std::string> &definitions) const;
		//std::optional<std::string> CompileShaders(prosper::ShaderStage stage,const std::string &shaderPath,std::string &outInfoLog,std::string &outDebugInfoLog) const;
		virtual bool InitializeShaderSources(prosper::Shader &shader, bool bReload, std::string &outInfoLog, std::string &outDebugInfoLog, prosper::ShaderStage &outErrStage, const std::string &prefixCode = {},
		  const std::unordered_map<std::string, std::string> &definitions = {}) const override;
		virtual std::optional<PipelineID> AddPipeline(prosper::Shader &shader, PipelineID shaderPipelineId, const prosper::ComputePipelineCreateInfo &createInfo, prosper::ShaderStageData &stage, PipelineID basePipelineId = std::numeric_limits<PipelineID>::max()) override;
		virtual std::optional<PipelineID> AddPipeline(prosper::Shader &shader, PipelineID shaderPipelineId, const prosper::RayTracingPipelineCreateInfo &createInfo, prosper::ShaderStageData &stage, PipelineID basePipelineId = std::numeric_limits<PipelineID>::max()) override
		{
			return {};
		} // Unsupported
		virtual std::optional<PipelineID> AddPipeline(prosper::Shader &shader, PipelineID shaderPipelineId, const prosper::GraphicsPipelineCreateInfo &createInfo, IRenderPass &rp, prosper::ShaderStageData *shaderStageFs = nullptr, prosper::ShaderStageData *shaderStageVs = nullptr,
		  prosper::ShaderStageData *shaderStageGs = nullptr, prosper::ShaderStageData *shaderStageTc = nullptr, prosper::ShaderStageData *shaderStageTe = nullptr, SubPassID subPassId = 0, PipelineID basePipelineId = std::numeric_limits<PipelineID>::max()) override;
		virtual bool ClearPipeline(bool graphicsShader, PipelineID pipelineId) override;

		virtual std::shared_ptr<prosper::IQueryPool> CreateQueryPool(QueryType queryType, uint32_t maxConcurrentQueries) override;
		virtual std::shared_ptr<prosper::IQueryPool> CreateQueryPool(QueryPipelineStatisticFlags statsFlags, uint32_t maxConcurrentQueries) override;
		virtual bool QueryResult(const TimestampQuery &query, std::chrono::nanoseconds &outTimestampValue) const override;
		virtual bool QueryResult(const PipelineStatisticsQuery &query, PipelineStatistics &outStatistics) const override;
		virtual bool QueryResult(const Query &query, uint32_t &r) const override;
		virtual bool QueryResult(const Query &query, uint64_t &r) const override;

		virtual void DrawFrame(const std::function<void()> &drawFrame) override;
		virtual bool Submit(ICommandBuffer &cmdBuf, bool shouldBlock = false, IFence *optFence = nullptr) override;
		virtual void Initialize(const CreateInfo &createInfo) override;

		ShaderBlit *GetBlitShader() const;
		ShaderFlipImage *GetFlipShader() const;

		virtual std::shared_ptr<IEvent> CreateEvent() override;
		virtual std::shared_ptr<IFence> CreateFence(bool createSignalled = false) override;
		virtual std::shared_ptr<ISampler> CreateSampler(const util::SamplerCreateInfo &createInfo) override;
		virtual std::shared_ptr<IRenderPass> CreateRenderPass(const util::RenderPassCreateInfo &renderPassInfo) override;
		virtual std::shared_ptr<IDescriptorSetGroup> CreateDescriptorSetGroup(DescriptorSetCreateInfo &descSetInfo) override;
		virtual std::shared_ptr<ISwapCommandBufferGroup> CreateSwapCommandBufferGroup(Window &window, bool allowMt = true, const std::string &debugName = {}) override;
		virtual std::shared_ptr<IFramebuffer> CreateFramebuffer(uint32_t width, uint32_t height, uint32_t layers, const std::vector<prosper::IImageView *> &attachments) override;
		virtual std::unique_ptr<IShaderPipelineLayout> GetShaderPipelineLayout(const Shader &shader, uint32_t pipelineIdx = 0u) const override;
		virtual std::shared_ptr<IRenderBuffer> CreateRenderBuffer(const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo, const std::vector<prosper::IBuffer *> &buffers, const std::vector<prosper::DeviceSize> &offsets = {},
		  const std::optional<IndexBufferInfo> &indexBufferInfo = {}) override;

		bool CheckResult();
		GLBuffer &GetPushConstantBuffer() const;
		std::optional<GLuint> GetPipelineProgram(PipelineID pipelineId) const;
		std::optional<uint32_t> ShaderPipelineDescSetBindingIndexToBindingPoint(PipelineID pipelineId, uint32_t setIdx, uint32_t bindingIdx) const;
		bool BindVertexBuffers(const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo, const std::vector<IBuffer *> &buffers, uint32_t startBinding, const std::vector<DeviceSize> &offsets, uint32_t *optOutAbsAttrId = nullptr);
	  protected:
		GLContext(const std::string &appName, bool bEnableValidation = false);
		virtual std::shared_ptr<IUniformResizableBuffer> DoCreateUniformResizableBuffer(const util::BufferCreateInfo &createInfo, uint64_t bufferInstanceSize, uint64_t maxTotalSize, const void *data, prosper::DeviceSize bufferBaseSize, uint32_t alignment) override;
		virtual std::shared_ptr<IImageView> DoCreateImageView(const util::ImageViewCreateInfo &createInfo, IImage &img, Format format, ImageViewType imgViewType, prosper::ImageAspectFlags aspectMask, uint32_t numLayers) override;
		virtual void DoKeepResourceAliveUntilPresentationComplete(const std::shared_ptr<void> &resource) override;
		virtual void DoWaitIdle() override;
		virtual void DoFlushCommandBuffer(ICommandBuffer &cmd) override;
		virtual void ReloadSwapchain() override;

		virtual void InitAPI(const CreateInfo &createInfo) override;
		void InitPushConstantBuffer();
		void InitShaderPipeline(prosper::Shader &shader, PipelineID pipelineId, PipelineID shaderPipelineId);
	  private:
		PipelineID AddPipeline(prosper::Shader &shader, PipelineID shaderPipelineId, std::shared_ptr<GLShaderProgram> program);
		struct PipelineData {
			std::shared_ptr<GLShaderProgram> program = nullptr;
			std::vector<std::vector<uint32_t>> descriptorSetBindingsToBindingPoints {};
		};
		::util::WeakHandle<Shader> m_hShaderBlit {};
		::util::WeakHandle<Shader> m_hShaderFlip {};
		std::shared_ptr<IBuffer> m_pushConstantBuffer = nullptr;
		std::vector<PipelineData> m_pipelines = {};
		std::queue<size_t> m_freePipelineIndices {};
		std::vector<std::shared_ptr<prosper::IFramebuffer>> m_swapchainFramebuffers {};
	};
};

#endif
