// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PR_PROSPER_GL_BUFFER_HPP__
#define __PR_PROSPER_GL_BUFFER_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_context_object.hpp>
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_buffer_create_info.hpp>
#include <mathutil/umath.h>
#include <memory>
#include <cinttypes>
#include <functional>

namespace prosper {
	class GLDynamicResizableBuffer;
	class GLUniformResizableBuffer;
	class DLLPROSPER_GL GLBuffer : virtual public prosper::IBuffer {
	  public:
		friend GLDynamicResizableBuffer;
		friend GLUniformResizableBuffer;
		static std::shared_ptr<IBuffer> Create(IPrContext &context, const util::BufferCreateInfo &bufCreateInfo, DeviceSize startOffset, GLuint bufIdx, const std::function<void(IBuffer &)> &onDestroyedCallback = nullptr);

		virtual ~GLBuffer() override;
		virtual std::shared_ptr<IBuffer> CreateSubBuffer(DeviceSize offset, DeviceSize size, const std::function<void(IBuffer &)> &onDestroyedCallback = nullptr) override;
		GLuint GetGLBuffer() const { return m_buffer; }
		virtual const void *GetInternalHandle() const override { return reinterpret_cast<void *>(GetGLBuffer()); }
	  private:
		GLBuffer(IPrContext &context, const util::BufferCreateInfo &bufCreateInfo, DeviceSize startOffset, DeviceSize size, GLuint bufIdx);
		bool ValidateBufferRange(DeviceSize offset, DeviceSize size) const;
		virtual bool DoWrite(Offset offset, Size size, const void *data) const override;
		virtual bool DoRead(Offset offset, Size size, void *data) const override;
		virtual bool DoMap(Offset offset, Size size, MapFlags mapFlags, void **optOutMappedPtr) const override;
		virtual bool DoUnmap() const override;

		GLuint m_buffer = GL_INVALID_VALUE;
		mutable DeviceSize m_mappedOffset = 0;
		mutable void *m_mappedPtr = nullptr;
	};
};

#endif
