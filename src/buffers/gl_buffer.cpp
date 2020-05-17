/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "buffers/gl_buffer.hpp"

using namespace prosper;

std::shared_ptr<IBuffer> GLBuffer::Create(IPrContext &context,const prosper::util::BufferCreateInfo &bufCreateInfo,DeviceSize startOffset,GLuint bufIdx)
{
	return std::shared_ptr<GLBuffer>{new GLBuffer{context,bufCreateInfo,startOffset,bufCreateInfo.size,bufIdx}};
}
GLBuffer::GLBuffer(IPrContext &context,const prosper::util::BufferCreateInfo &bufCreateInfo,DeviceSize startOffset,DeviceSize size,GLuint bufIdx)
	: IBuffer{context,bufCreateInfo,startOffset,size},m_buffer{bufIdx}
{}
std::shared_ptr<IBuffer> GLBuffer::CreateSubBuffer(DeviceSize offset,DeviceSize size,const std::function<void(IBuffer&)> &onDestroyedCallback)
{
	auto subBuffer = Create(GetContext(),m_createInfo,(m_parent ? m_parent->GetStartOffset() : 0ull) +offset,m_buffer);
	static_cast<GLBuffer*>(subBuffer.get())->SetParent(*this);
	return subBuffer;
}
bool GLBuffer::DoMap(Offset offset,Size size) const
{
	m_mappedPtr = glMapBufferRange(m_buffer,GetStartOffset() +offset,size,GL_MAP_WRITE_BIT);
	return m_mappedPtr;
}
bool GLBuffer::DoUnmap() const
{
	return glUnmapBuffer(m_buffer);
}

bool GLBuffer::DoWrite(Offset offset,Size size,const void *data) const
{
	auto *ptr = glMapBufferRange(m_buffer,GetStartOffset() +offset,size,GL_MAP_WRITE_BIT);
	if(ptr == nullptr)
		return false;
	memcpy(ptr,data,size);
	return glUnmapBuffer(m_buffer);
}
bool GLBuffer::DoRead(Offset offset,Size size,void *data) const
{
	auto *ptr = glMapBufferRange(m_buffer,GetStartOffset() +offset,size,GL_MAP_READ_BIT);
	if(ptr == nullptr)
		return false;
	memcpy(data,ptr,size);
	return glUnmapBuffer(m_buffer);
}

GLBuffer::~GLBuffer()
{
	if(GetParent() == nullptr)
		glDeleteBuffers(1,&m_buffer);
}