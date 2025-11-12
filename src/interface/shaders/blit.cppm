// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.prosper.opengl:shader.blit;

export import pragma.prosper;

export namespace prosper {
	class PR_EXPORT ShaderBlit : public ShaderBaseImageProcessing {
	  public:
		ShaderBlit(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture) const;

		// Expects the texture to already be bound
		using ShaderBaseImageProcessing::RecordDraw;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
	};
};
