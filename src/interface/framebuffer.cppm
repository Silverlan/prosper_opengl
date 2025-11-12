// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "opengl_api.hpp"

export module pragma.prosper.opengl:framebuffer;

export import pragma.prosper;

export namespace prosper {
	class GLContext;
	class GLWindow;
	class PR_EXPORT GLFramebuffer : public prosper::IFramebuffer {
	  public:
		static std::shared_ptr<IFramebuffer> Create(IPrContext &context, const std::vector<std::shared_ptr<IImageView>> &attachments, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers);
		friend GLContext;
		friend GLWindow;

		virtual ~GLFramebuffer() override;
		GLuint GetGLFramebuffer() const { return m_framebuffer; }
		virtual const void *GetInternalHandle() const override;

		void UpateSize(uint32_t w, uint32_t h);
	  private:
		GLFramebuffer(IPrContext &context, const std::vector<std::shared_ptr<IImageView>> &attachments, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, GLuint framebuffer);

		GLuint m_framebuffer = GL_INVALID_VALUE;
	};
};
