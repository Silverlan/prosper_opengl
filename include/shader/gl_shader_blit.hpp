// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PROSPER_GL_SHADER_BLIT_HPP__
#define __PROSPER_GL_SHADER_BLIT_HPP__

#include "gl_api.hpp"
#include <shader/prosper_shader_base_image_processing.hpp>
#include <mathutil/uvec.h>

namespace prosper {
	class RenderTarget;
	class DLLPROSPER_GL ShaderBlit : public ShaderBaseImageProcessing {
	  public:
		ShaderBlit(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture) const;

		// Expects the texture to already be bound
		using ShaderBaseImageProcessing::RecordDraw;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
	};
};

#endif
