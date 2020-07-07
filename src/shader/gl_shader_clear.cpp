/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "shader/gl_shader_clear.hpp"
#include "shader/prosper_pipeline_create_info.hpp"
#include <prosper_util_square_shape.hpp>

using namespace prosper;

decltype(ShaderClear::VERTEX_BINDING_VERTEX) ShaderClear::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderClear::VERTEX_ATTRIBUTE_POSITION) ShaderClear::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,prosper::util::get_square_vertex_format()};

ShaderClear::ShaderClear(prosper::IPrContext &context,const std::string &identifier)
	: ShaderGraphics(context,identifier,"screen/vs_screen","screen/fs_clear")
{}

void ShaderClear::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(pipelineInfo,0,sizeof(Vector4),prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderClear::Draw()
{
	auto vertBuffer = prosper::util::get_square_vertex_buffer(GetContext());
	if(
		RecordBindVertexBuffers({vertBuffer.get()}) == false ||
		RecordDraw(prosper::util::get_square_vertex_count()) == false
		)
		return false;
	return true;
}

bool ShaderClear::Draw(const Vector4 &color) {return RecordPushConstants(color) && Draw();}
