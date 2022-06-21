/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "buffers/gl_render_buffer.hpp"
#include "buffers/gl_buffer.hpp"
#include "gl_api.hpp"
#include "gl_context.hpp"

using namespace prosper;

GLRenderBuffer::GLRenderBuffer(
	prosper::IPrContext &context,const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo,const std::vector<prosper::IBuffer*> &buffers,const std::vector<prosper::DeviceSize> &offsets,const std::optional<IndexBufferInfo> &indexBufferInfo
)
	: IRenderBuffer{context,pipelineCreateInfo,buffers,offsets,indexBufferInfo},
	m_vao{std::numeric_limits<decltype(m_vao)>::max()}
{}
GLRenderBuffer::~GLRenderBuffer()
{
	glDeleteVertexArrays(1,&m_vao);
}
GLuint GLRenderBuffer::GetGLVertexArrayObject() const {return m_vao;}
void GLRenderBuffer::Reload()
{
	GLint oldVao;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldVao);
	glBindVertexArray(m_vao);
	std::vector<prosper::IBuffer*> buffers;
	buffers.reserve(m_buffers.size());
	for(auto &buf : m_buffers)
		buffers.push_back(buf.get());
	static_cast<GLContext&>(GetContext()).BindVertexBuffers(GetPipelineCreateInfo(),buffers,0u,m_offsets);
	if(m_indexBufferInfo.has_value())
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_indexBufferInfo->buffer->GetAPITypeRef<GLBuffer>().GetGLBuffer());
	glBindVertexArray(oldVao);
}
std::shared_ptr<GLRenderBuffer> GLRenderBuffer::Create(
	prosper::GLContext &context,const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo,const std::vector<prosper::IBuffer*> &buffers,
	const std::vector<prosper::DeviceSize> &offsets,const std::optional<IndexBufferInfo> &indexBufferInfo
)
{
	GLuint vao = std::numeric_limits<GLuint>::max();
	glCreateVertexArrays(1,&vao);

	auto buf = std::shared_ptr<GLRenderBuffer>{new GLRenderBuffer{context,pipelineCreateInfo,buffers,offsets,indexBufferInfo}};
	buf->m_vao = vao;
	buf->Reload();
	return buf;
}
