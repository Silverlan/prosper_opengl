/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PROSPER_GL_SHADER_FLIP_Y_HPP__
#define __PROSPER_GL_SHADER_FLIP_Y_HPP__

#include "gl_api.hpp"
#include <shader/prosper_shader_base_image_processing.hpp>
#include <mathutil/uvec.h>

namespace prosper
{
	class RenderTarget;
	class DLLPROSPER_GL ShaderFlipY
		: public ShaderBaseImageProcessing
	{
	public:
		ShaderFlipY(prosper::IPrContext &context,const std::string &identifier);
		bool RecordDraw(ShaderBindState &bindState,prosper::IDescriptorSet &descSetTexture) const;

		// Expects the texture to already be bound
		using ShaderBaseImageProcessing::RecordDraw;
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
