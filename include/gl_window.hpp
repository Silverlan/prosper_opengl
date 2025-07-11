// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __GL_WINDOW_HPP__
#define __GL_WINDOW_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_window.hpp>

namespace prosper {
	class GLContext;
	class GLImage;
	class GLFramebuffer;
	class DLLPROSPER_GL GLWindow : public Window {
	  public:
		static std::shared_ptr<GLWindow> Create(const WindowSettings &windowCreationInfo, prosper::IPrContext &context);
		virtual ~GLWindow() override {}
		virtual uint32_t GetLastAcquiredSwapchainImageIndex() const override { return m_lastAcquiredSwapchainImageIndex; }
	  protected:
		friend GLContext;
		using Window::Window;
		void InitWindow(bool keepContext);
		virtual void InitWindow() override;
		virtual void ReleaseWindow() override;
		virtual void DoInitSwapchain() override;
		virtual void DoReleaseSwapchain() override;
		virtual void InitCommandBuffers() override;
		virtual void DoReloadWindow() override;

		uint32_t m_lastAcquiredSwapchainImageIndex = 0;
	};
};

#endif
