/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __GL_WINDOW_HPP__
#define __GL_WINDOW_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_window.hpp>

namespace prosper
{
	class GLContext;
	class GLImage;
	class GLFramebuffer;
	class DLLPROSPER_GL GLWindow
		: public Window
	{
	public:
		static std::shared_ptr<GLWindow> Create(const WindowSettings &windowCreationInfo,prosper::IPrContext &context);
		virtual ~GLWindow() override {}
		virtual uint32_t GetLastAcquiredSwapchainImageIndex() const override {return m_lastAcquiredSwapchainImageIndex;}
	protected:
		friend GLContext;
		using Window::Window;
		virtual void InitWindow() override;
		virtual void ReleaseWindow() override;
		virtual void DoInitSwapchain() override;
		virtual void DoReleaseSwapchain() override;
		virtual void InitCommandBuffers() override;

		uint32_t m_lastAcquiredSwapchainImageIndex = 0;
	};
};

#endif
