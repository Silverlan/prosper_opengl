// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.prosper.opengl:window;

export import pragma.prosper;

export namespace prosper {
	class GLContext;
	class GLImage;
	class GLFramebuffer;
	class PR_EXPORT GLWindow : public Window {
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
