// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PR_PROSPER_GL_RENDER_BUFFER_HPP__
#define __PR_PROSPER_GL_RENDER_BUFFER_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_context_object.hpp>
#include <buffers/prosper_render_buffer.hpp>

namespace prosper {
	class GLContext;
	class GraphicsPipelineCreateInfo;
	class DLLPROSPER_GL GLRenderBuffer : public prosper::IRenderBuffer {
	  public:
		static std::shared_ptr<GLRenderBuffer> Create(prosper::GLContext &context, const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo, const std::vector<prosper::IBuffer *> &buffers, const std::vector<prosper::DeviceSize> &offsets = {},
		  const std::optional<IndexBufferInfo> &indexBufferInfo = {});
		virtual ~GLRenderBuffer() override;
		GLuint GetGLVertexArrayObject() const;
	  private:
		GLRenderBuffer(prosper::IPrContext &context, const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo, const std::vector<prosper::IBuffer *> &buffers, const std::vector<prosper::DeviceSize> &offsets, const std::optional<IndexBufferInfo> &indexBufferInfo = {});
		virtual void Reload() override;
		GLuint m_vao;
	};
};

#endif
