// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "opengl_api.hpp"

export module pragma.prosper.opengl:buffer.render_buffer;

export import pragma.prosper;

export namespace prosper {
	class GLContext;
	class PR_EXPORT GLRenderBuffer : public prosper::IRenderBuffer {
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
