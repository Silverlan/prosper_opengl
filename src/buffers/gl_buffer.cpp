// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "buffers/gl_buffer.hpp"
#include "gl_api.hpp"
#include "gl_context.hpp"

using namespace prosper;

std::shared_ptr<IBuffer> GLBuffer::Create(IPrContext &context, const prosper::util::BufferCreateInfo &bufCreateInfo, DeviceSize startOffset, GLuint bufIdx, const std::function<void(IBuffer &)> &onDestroyedCallback)
{
	return std::shared_ptr<GLBuffer> {new GLBuffer {context, bufCreateInfo, startOffset, bufCreateInfo.size, bufIdx}, [onDestroyedCallback](GLBuffer *buf) {
		                                  if(onDestroyedCallback)
			                                  onDestroyedCallback(*buf);
		                                  delete buf;
	                                  }};
}
GLBuffer::GLBuffer(IPrContext &context, const prosper::util::BufferCreateInfo &bufCreateInfo, DeviceSize startOffset, DeviceSize size, GLuint bufIdx) : IBuffer {context, bufCreateInfo, startOffset, size}, m_buffer {bufIdx} { m_apiTypePtr = this; }
std::shared_ptr<IBuffer> GLBuffer::CreateSubBuffer(DeviceSize offset, DeviceSize size, const std::function<void(IBuffer &)> &onDestroyedCallback)
{
	auto subBufferCreateInfo = m_createInfo;
	subBufferCreateInfo.size = size;
	auto subBuffer = Create(GetContext(), subBufferCreateInfo, (m_parent ? m_parent->GetStartOffset() : 0ull) + offset, m_buffer, onDestroyedCallback);
	subBuffer->GetAPITypeRef<GLBuffer>().SetParent(*this);
	return subBuffer;
}
bool GLBuffer::DoMap(Offset offset, Size size, MapFlags mapFlags, void **optOutMappedPtr) const
{
	if(m_permanentlyMapped.has_value()) {
		if(optOutMappedPtr)
			*optOutMappedPtr = static_cast<uint8_t *>(m_mappedPtr) + offset;
		m_mappedOffset = offset;
		return true;
	}
	GLbitfield access = 0;
	auto &createInfo = GetCreateInfo();
	/*if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::ReadOnly) == false)
		access |= GL_MAP_WRITE_BIT;
	if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::HostAccessable))
		access |= GL_MAP_READ_BIT;*/
	if(umath::is_flag_set(mapFlags, MapFlags::ReadBit))
		access |= GL_MAP_READ_BIT;
	if(umath::is_flag_set(mapFlags, MapFlags::WriteBit))
		access |= GL_MAP_WRITE_BIT;
	if(umath::is_flag_set(mapFlags, MapFlags::Unsynchronized))
		access |= GL_MAP_UNSYNCHRONIZED_BIT;
	if(umath::is_flag_set(createInfo.memoryFeatures, prosper::MemoryFeatureFlags::HostCoherent))
		access |= GL_MAP_COHERENT_BIT;
	if(umath::is_flag_set(mapFlags, MapFlags::PersistentBit)) {
		if(umath::is_flag_set(createInfo.flags, util::BufferCreateInfo::Flags::Persistent) == false)
			throw std::logic_error {"Attempted to map buffer persistently, but buffer has not been created with persistent flag!"};
		access |= GL_MAP_PERSISTENT_BIT;
	}
	if(static_cast<GLContext &>(GetContext()).IsValidationEnabled()) {
		GLint mapped = GL_FALSE;
		glGetNamedBufferParameteriv(m_buffer, GL_BUFFER_MAPPED, &mapped);
		if(mapped)
			GetContext().ValidationCallback(prosper::DebugMessageSeverityFlags::WarningBit, "Attempted to map buffer that was already mapped, which is not allowed!");
	}
	ValidateBufferRange(offset, size);
	m_mappedPtr = glMapNamedBufferRange(m_buffer, GetStartOffset() + offset, size, access);
	if(static_cast<GLContext &>(GetContext()).IsValidationEnabled()) {
		auto result = static_cast<GLContext &>(GetContext()).CheckResult();
		if(result == false) {
			GLint createAccessFlags = 0;
			glGetNamedBufferParameteriv(m_buffer, GL_BUFFER_STORAGE_FLAGS, &createAccessFlags);
			if((access & createAccessFlags) != access)
				GetContext().ValidationCallback(prosper::DebugMessageSeverityFlags::WarningBit, "Buffer mapping requested access flags " + std::to_string(access) + ", which is not compatible with access flags " + std::to_string(createAccessFlags) + " that the buffer was created with!");

			GLint64 size = 0;
			glGetNamedBufferParameteri64v(m_buffer, GL_BUFFER_SIZE, &size);
			if(GetStartOffset() + offset + size >= size)
				GetContext().ValidationCallback(prosper::DebugMessageSeverityFlags::WarningBit, "Map range for buffer exceeds buffer range!");
		}
	}
	if(optOutMappedPtr)
		*optOutMappedPtr = m_mappedPtr;
	return m_mappedPtr;
}
bool GLBuffer::DoUnmap() const
{
	if(m_permanentlyMapped.has_value()) {
		m_mappedOffset = 0;
		return true;
	}
	m_mappedPtr = nullptr;
	return glUnmapNamedBuffer(m_buffer);
}

bool GLBuffer::DoWrite(Offset offset, Size size, const void *data) const
{
	ValidateBufferRange(m_mappedOffset + offset, size);
	if(m_mappedPtr) {
		memcpy(static_cast<uint8_t *>(m_mappedPtr) + m_mappedOffset + offset, data, size);
		return true;
	}
	if(Map(offset, size, prosper::IBuffer::MapFlags::WriteBit) == false)
		return false;
	memcpy(static_cast<uint8_t *>(m_mappedPtr), data, size);
	return Unmap();
}

bool GLBuffer::DoRead(Offset offset, Size size, void *data) const
{
	ValidateBufferRange(m_mappedOffset + offset, size);
	if(m_mappedPtr) {
		memcpy(data, static_cast<uint8_t *>(m_mappedPtr) + m_mappedOffset + offset, size);
		return true;
	}
	if(Map(offset, size, prosper::IBuffer::MapFlags::ReadBit) == false)
		return false;
	memcpy(data, static_cast<uint8_t *>(m_mappedPtr), size);
	return Unmap();
}

bool GLBuffer::ValidateBufferRange(DeviceSize offset, DeviceSize size) const
{
	offset += GetStartOffset();
	auto &context = static_cast<GLContext &>(GetContext());
	if(context.IsValidationEnabled() == false)
		return true;
	GLint64 bufSize;
	glGetNamedBufferParameteri64v(m_buffer, GL_BUFFER_SIZE, &bufSize);
	if(context.CheckResult() == false)
		return false;
	if(offset + size > bufSize) {
		context.ValidationCallback(prosper::DebugMessageSeverityFlags::ErrorBit, "Attempted to access buffer range [" + std::to_string(offset) + "," + std::to_string(offset + size) + "], which exceeds buffer size of " + std::to_string(bufSize));
		return false;
	}
	return true;
}

GLBuffer::~GLBuffer()
{
	if(GetParent() == nullptr && m_buffer != 0)
		glDeleteBuffers(1, &m_buffer);
}
