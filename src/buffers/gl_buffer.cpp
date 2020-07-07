/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "buffers/gl_buffer.hpp"
#include "gl_api.hpp"
#include "gl_context.hpp"

using namespace prosper;

#pragma optimize("",off)
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
	dynamic_cast<GLBuffer*>(subBuffer.get())->SetParent(*this);
	return subBuffer;
}
bool GLBuffer::DoMap(Offset offset,Size size,MapFlags mapFlags) const
{
	GLbitfield access = 0;
	auto &createInfo = GetCreateInfo();
	if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::ReadOnly) == false)
		access |= GL_MAP_WRITE_BIT;
	if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::HostAccessable))
		access |= GL_MAP_READ_BIT;
	if(umath::is_flag_set(createInfo.memoryFeatures,prosper::MemoryFeatureFlags::HostCoherent))
		access |= GL_MAP_COHERENT_BIT;
	if(umath::is_flag_set(mapFlags,MapFlags::PersistentBit))
	{
		if(umath::is_flag_set(createInfo.flags,util::BufferCreateInfo::Flags::Persistent) == false)
			throw std::logic_error{"Attempted to map buffer persistently, but buffer has not been created with persistent flag!"};
		access |= GL_MAP_PERSISTENT_BIT;
	}
	m_mappedPtr = glMapNamedBufferRange(m_buffer,GetStartOffset() +offset,size,access);
	return m_mappedPtr;
}
bool GLBuffer::DoUnmap() const
{
	m_mappedPtr = nullptr;
	return glUnmapNamedBuffer(m_buffer);
}

bool GLBuffer::DoWrite(Offset offset,Size size,const void *data) const
{
	if(m_mappedPtr)
	{
		memcpy(static_cast<uint8_t*>(m_mappedPtr) +offset,data,size);
		return true;
	}
	if(Map(offset,size,prosper::IBuffer::MapFlags::None) == false)
		return false;
	memcpy(static_cast<uint8_t*>(m_mappedPtr) +offset,data,size);
	return Unmap();
}

bool GLBuffer::DoRead(Offset offset,Size size,void *data) const
{
	if(m_mappedPtr)
	{
		memcpy(data,static_cast<uint8_t*>(m_mappedPtr) +offset,size);
		return true;
	}
	if(Map(offset,size,prosper::IBuffer::MapFlags::None) == false)
		return false;
	memcpy(data,static_cast<uint8_t*>(m_mappedPtr) +offset,size);
	return Unmap();
}

GLBuffer::~GLBuffer()
{
	if(GetParent() == nullptr && m_buffer != 0)
		glDeleteBuffers(1,&m_buffer);
}
#pragma optimize("",on)
