// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PR_PROSPER_GL_DYNAMIC_RESIZABLE_BUFFER_HPP__
#define __PR_PROSPER_GL_DYNAMIC_RESIZABLE_BUFFER_HPP__

#include "prosper_definitions.hpp"
#include "prosper_includes.hpp"
#include "gl_buffer.hpp"
#include "buffers/prosper_dynamic_resizable_buffer.hpp"

namespace prosper {
	class DLLPROSPER_GL GLDynamicResizableBuffer : public IDynamicResizableBuffer, virtual public GLBuffer {
	  public:
		GLDynamicResizableBuffer(IPrContext &context, IBuffer &buffer, const util::BufferCreateInfo &createInfo, uint64_t maxTotalSize);
	  protected:
		virtual void MoveInternalBuffer(IBuffer &other) override;
	};
};

#endif
