/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "shader/gl_shader_blit.hpp"
#include "shader/prosper_pipeline_create_info.hpp"

using namespace prosper;

ShaderBlit::ShaderBlit(prosper::IPrContext &context,const std::string &identifier)
	: ShaderBaseImageProcessing(context,identifier,"screen/vs_screen_uv","screen/fs_blit")
{}

void ShaderBlit::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
}

bool ShaderBlit::RecordDraw(ShaderBindState &bindState,prosper::IDescriptorSet &descSetTexture) const {return ShaderBaseImageProcessing::RecordDraw(bindState,descSetTexture);}
