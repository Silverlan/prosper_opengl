// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.prosper.opengl;

import :buffer.resizable_buffer;

using namespace prosper;

prosper::GLResizableBuffer::GLResizableBuffer(IBuffer &buffer)
    : IResizableBuffer {buffer}, IBuffer {buffer.GetContext(), buffer.GetCreateInfo(), buffer.GetStartOffset(), buffer.GetSize()}, GLBuffer {buffer.GetContext(), buffer.GetCreateInfo(), buffer.GetStartOffset(), buffer.GetSize(), 0}
{
	GLBuffer::m_buffer = buffer.GetAPITypeRef<GLBuffer>().m_buffer;
	buffer.GetAPITypeRef<GLBuffer>().m_buffer = 0; // Setting to 0 to ensure it won't get deleted
}

void prosper::GLResizableBuffer::MoveInternalBuffer(IBuffer &other)
{
	auto &glBuffer = other.GetAPITypeRef<GLBuffer>();
	m_buffer = glBuffer.m_buffer;
	glBuffer.m_buffer = 0;
}
