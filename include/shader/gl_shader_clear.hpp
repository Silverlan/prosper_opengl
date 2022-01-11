/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PROSPER_GL_SHADER_CLEAR_HPP__
#define __PROSPER_GL_SHADER_CLEAR_HPP__

#include "gl_api.hpp"
#include <shader/prosper_shader.hpp>
#include <mathutil/uvec.h>

namespace prosper
{
	class RenderTarget;
	class DLLPROSPER_GL ShaderClear
		: public ShaderGraphics
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

		ShaderClear(prosper::IPrContext &context,const std::string &identifier);
		bool RecordDraw(ShaderBindState &bindState,const Vector4 &color) const;
	protected:
		virtual bool RecordDraw(ShaderBindState &bindState) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
