/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PR_PROSPER_GL_RENDER_BUFFER_HPP__
#define __PR_PROSPER_GL_RENDER_BUFFER_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_context_object.hpp>
#include <buffers/prosper_render_buffer.hpp>

namespace prosper
{
	class GLContext;
	class GraphicsPipelineCreateInfo;
	class DLLPROSPER_GL GLRenderBuffer
		: public prosper::IRenderBuffer
	{
	public:
		static std::shared_ptr<GLRenderBuffer> Create(
			prosper::GLContext &context,const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo,const std::vector<prosper::IBuffer*> &buffers,
			const std::vector<prosper::DeviceSize> &offsets={},const std::optional<IndexBufferInfo> &indexBufferInfo={}
		);
		virtual ~GLRenderBuffer() override;
		GLuint GetGLVertexArrayObject() const;
	private:
		GLRenderBuffer(
			prosper::IPrContext &context,const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo,const std::vector<prosper::IBuffer*> &buffers,const std::vector<prosper::DeviceSize> &offsets,const std::optional<IndexBufferInfo> &indexBufferInfo={}
		);
		virtual void Reload() override;
		GLuint m_vao;
	};
};

#endif
