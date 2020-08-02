/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PROSPER_GL_FRAMEBUFFER_HPP__
#define __PROSPER_GL_FRAMEBUFFER_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_framebuffer.hpp>

namespace prosper
{
	class GLContext;
	class DLLPROSPER_GL GLFramebuffer
		: public prosper::IFramebuffer
	{
	public:
		static std::shared_ptr<IFramebuffer> Create(
			IPrContext &context,const std::vector<std::shared_ptr<IImageView>> &attachments,
			uint32_t width,uint32_t height,uint32_t depth,uint32_t layers
		);
		friend GLContext;

		virtual ~GLFramebuffer() override;
		GLuint GetGLFramebuffer() const {return m_framebuffer;}
		virtual const void *GetInternalHandle() const override;

		void UpateSize(uint32_t w,uint32_t h);
	private:
		GLFramebuffer(
			IPrContext &context,const std::vector<std::shared_ptr<IImageView>> &attachments,
			uint32_t width,uint32_t height,uint32_t depth,uint32_t layers,GLuint framebuffer
		);

		GLuint m_framebuffer = GL_INVALID_VALUE;
	};
};

#endif
