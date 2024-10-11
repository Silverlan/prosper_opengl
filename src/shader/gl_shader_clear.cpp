/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "shader/gl_shader_clear.hpp"
#include "shader/prosper_pipeline_create_info.hpp"
#include <prosper_context.hpp>

using namespace prosper;

decltype(ShaderClear::VERTEX_BINDING_VERTEX) ShaderClear::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderClear::VERTEX_ATTRIBUTE_POSITION) ShaderClear::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,CommonBufferCache::GetSquareVertexFormat()};

ShaderClear::ShaderClear(prosper::IPrContext &context,const std::string &identifier)
	: ShaderGraphics(context,identifier,"screen/vs_screen","screen/fs_clear")
{}

void ShaderClear::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(0,sizeof(Vector4),prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderClear::RecordDraw(ShaderBindState &bindState) const
{
	auto vertBuffer = GetContext().GetCommonBufferCache().GetSquareVertexBuffer();
	if(
		RecordBindVertexBuffers(bindState,{vertBuffer.get()}) == false ||
		ShaderGraphics::RecordDraw(bindState,GetContext().GetCommonBufferCache().GetSquareVertexCount()) == false
		)
		return false;
	return true;
}

bool ShaderClear::RecordDraw(ShaderBindState &bindState,const Vector4 &color) const {return RecordPushConstants(bindState,color) && RecordDraw(bindState);}
