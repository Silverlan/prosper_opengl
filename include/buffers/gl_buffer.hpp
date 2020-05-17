/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

namespace prosper
{
	class DLLPROSPER_GL GLBuffer
		: public prosper::IBuffer
	{
	public:
		static std::shared_ptr<IBuffer> Create(IPrContext &context,const util::BufferCreateInfo &bufCreateInfo,DeviceSize startOffset,GLuint bufIdx);

		virtual ~GLBuffer() override;
		virtual std::shared_ptr<IBuffer> CreateSubBuffer(DeviceSize offset,DeviceSize size,const std::function<void(IBuffer&)> &onDestroyedCallback=nullptr) override;
		GLuint GetGLBuffer() const {return m_buffer;}
	private:
		GLBuffer(IPrContext &context,const util::BufferCreateInfo &bufCreateInfo,DeviceSize startOffset,DeviceSize size,GLuint bufIdx);
		virtual bool DoWrite(Offset offset,Size size,const void *data) const override;
		virtual bool DoRead(Offset offset,Size size,void *data) const override;
		virtual bool DoMap(Offset offset,Size size) const override;
		virtual bool DoUnmap() const override;

		GLuint m_buffer = GL_INVALID_VALUE;
		mutable void *m_mappedPtr = nullptr;
	};
};

#endif
