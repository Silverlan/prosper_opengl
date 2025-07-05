// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "prosper_includes.hpp"
#include "buffers/gl_uniform_resizable_buffer.hpp"
#include "prosper_util.hpp"
#include "buffers/prosper_buffer.hpp"

using namespace prosper;

prosper::GLUniformResizableBuffer::GLUniformResizableBuffer(IPrContext &context, IBuffer &buffer, uint64_t bufferInstanceSize, uint64_t alignedBufferBaseSize, uint64_t maxTotalSize, uint32_t alignment)
    : IUniformResizableBuffer {context, buffer, bufferInstanceSize, alignedBufferBaseSize, maxTotalSize, alignment}, IBuffer {buffer.GetContext(), buffer.GetCreateInfo(), buffer.GetStartOffset(), buffer.GetSize()},
      GLBuffer {buffer.GetContext(), buffer.GetCreateInfo(), buffer.GetStartOffset(), buffer.GetSize(), 0}
{
	GLBuffer::m_buffer = buffer.GetAPITypeRef<GLBuffer>().m_buffer;
	buffer.GetAPITypeRef<GLBuffer>().m_buffer = 0; // Setting to 0 to ensure it won't get deleted
}

void prosper::GLUniformResizableBuffer::MoveInternalBuffer(IBuffer &other)
{
	auto &glBuffer = other.GetAPITypeRef<GLBuffer>();
	m_buffer = glBuffer.m_buffer;
	glBuffer.m_buffer = 0;
}
