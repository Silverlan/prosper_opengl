/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "prosper_includes.hpp"
#include "buffers/gl_uniform_resizable_buffer.hpp"
#include "prosper_util.hpp"
#include "buffers/prosper_buffer.hpp"
#include "vk_context.hpp"

using namespace prosper;

prosper::GLUniformResizableBuffer::GLUniformResizableBuffer(
	IPrContext &context,IBuffer &buffer,
	uint64_t bufferInstanceSize,
	uint64_t alignedBufferBaseSize,uint64_t maxTotalSize,uint32_t alignment
)
	: IUniformResizableBuffer{context,buffer,bufferInstanceSize,alignedBufferBaseSize,maxTotalSize,alignment},
	IBuffer{buffer.GetContext(),buffer.GetCreateInfo(),buffer.GetStartOffset(),buffer.GetSize()},
	GLBuffer{buffer.GetContext(),buffer.GetCreateInfo(),buffer.GetStartOffset(),buffer.GetSize(),0}
{
	GLBuffer::m_buffer = dynamic_cast<GLBuffer&>(buffer).m_buffer;
	dynamic_cast<GLBuffer&>(buffer).m_buffer = 0; // Setting to 0 to ensure it won't get deleted
}
