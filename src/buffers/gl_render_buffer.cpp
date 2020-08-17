/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "buffers/gl_render_buffer.hpp"
#include "buffers/gl_buffer.hpp"
#include "gl_api.hpp"
#include "gl_context.hpp"

using namespace prosper;

#pragma optimize("",off)
GLRenderBuffer::GLRenderBuffer(
	prosper::IPrContext &context,const std::vector<prosper::IBuffer*> &buffers,const std::optional<IndexBufferInfo> &indexBufferInfo
)
	: IRenderBuffer{context,buffers,indexBufferInfo}
{}
GLRenderBuffer::~GLRenderBuffer()
{
	glDeleteVertexArrays(1,&m_vao);
}
GLuint GLRenderBuffer::GetGLVertexArrayObject() const {return m_vao;}
std::shared_ptr<GLRenderBuffer> GLRenderBuffer::Create(
	prosper::GLContext &context,const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo,const std::vector<prosper::IBuffer*> &buffers,
	const std::vector<prosper::DeviceSize> &offsets,const std::optional<IndexBufferInfo> &indexBufferInfo
)
{
	GLint oldVao;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldVao);
	GLuint vao;
	glCreateVertexArrays(1,&vao);
	glBindVertexArray(vao);
	context.BindVertexBuffers(pipelineCreateInfo,buffers,0u,offsets);
	if(indexBufferInfo.has_value())
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,dynamic_cast<prosper::GLBuffer&>(*indexBufferInfo->buffer).GetGLBuffer());
	glBindVertexArray(oldVao);

	auto buf = std::shared_ptr<GLRenderBuffer>{new GLRenderBuffer{context,buffers,indexBufferInfo}};
	buf->m_vao = vao;
	return buf;
}
#pragma optimize("",on)
