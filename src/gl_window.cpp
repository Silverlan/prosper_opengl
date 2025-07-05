// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "gl_context.hpp"
#include "gl_window.hpp"
#include "image/gl_image.hpp"
#include "image/gl_image_view.hpp"
#include "gl_framebuffer.hpp"
#include "gl_api.hpp"
#include <thread>
#include "gl_command_buffer.hpp"

using namespace prosper;

std::shared_ptr<GLWindow> GLWindow::Create(const WindowSettings &windowCreationInfo, prosper::IPrContext &context)
{
	auto window = std::shared_ptr<GLWindow> {new GLWindow {context, windowCreationInfo}, [](GLWindow *window) {
		                                         window->Release();
		                                         delete window;
	                                         }};
	window->InitWindow();
	return window;
}

void GLWindow::InitWindow(bool keepContext)
{
	if(!keepContext)
		m_glfwWindow = {};

	// TODO: Clean this up
	pragma::platform::poll_events();
	auto settings = m_settings;
	if(!settings.windowedMode && (settings.monitor.has_value() == false || settings.monitor->GetGLFWMonitor() == nullptr))
		settings.monitor = pragma::platform::get_primary_monitor();
	settings.api = pragma::platform::WindowCreationInfo::API::OpenGL;
	if(GetContext().IsValidationEnabled())
		settings.flags |= pragma::platform::WindowCreationInfo::Flags::DebugContext;
	if(m_glfwWindow) {
		// Don't re-create the window (which would destroy the OpenGL context),
		// just try to re-initialize it with the new settings.
		m_glfwWindow->Reinitialize(settings);
	}
	else
		m_glfwWindow = pragma::platform::Window::Create(settings); // TODO: Release

	const char *errDesc;
	auto err = glfwGetError(&errDesc);
	if(err != GLFW_NO_ERROR) {
		std::cout << "Error retrieving GLFW window handle: " << errDesc << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		exit(EXIT_FAILURE);
	}

	// GLFW does not guarantee to actually use the size which was specified,
	// in some cases it may change, so we have to retrieve it again here
	auto actualWindowSize = m_glfwWindow->GetSize();
	m_settings.width = actualWindowSize.x;
	m_settings.height = actualWindowSize.y;

	m_glfwWindow->SetResizeCallback([](pragma::platform::Window &window, Vector2i size) {
		std::cout << "Resizing..." << std::endl; // TODO
	});
	glfwMakeContextCurrent(const_cast<GLFWwindow *>(m_glfwWindow->GetGLFWWindow()));

	OnWindowInitialized();
}
void GLWindow::InitWindow() { InitWindow(false); }
void GLWindow::ReleaseWindow()
{
	ReleaseSwapchain();
	m_glfwWindow = nullptr;
}
void GLWindow::DoInitSwapchain()
{
	auto size = m_glfwWindow->GetSize();
	util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.width = size.x;
	imgCreateInfo.height = size.y;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.layers = 1;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	imgCreateInfo.tiling = prosper::ImageTiling::Optimal;
	imgCreateInfo.usage = prosper::ImageUsageFlags::TransferDstBit;
	imgCreateInfo.flags = util::ImageCreateInfo::Flags::DontAllocateMemory;
	auto img = std::shared_ptr<GLImage> {new GLImage {GetContext(), imgCreateInfo, 0, GL_RGBA}};
	m_swapchainImages = {img, img};

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.baseLayer = 0;
	imgViewCreateInfo.baseMipmap = 0;
	imgViewCreateInfo.mipmapLevels = 1;
	imgViewCreateInfo.levelCount = 1;
	imgViewCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	auto imgView = GLImageView::Create(GetContext(), *img, imgViewCreateInfo, prosper::ImageViewType::e2D, prosper::ImageAspectFlags::ColorBit);
	auto framebuffer = std::shared_ptr<GLFramebuffer> {new GLFramebuffer {GetContext(), {imgView}, imgCreateInfo.width, imgCreateInfo.height, 1, 1, 0}};
	m_swapchainFramebuffers = {framebuffer, framebuffer};

	OnSwapchainInitialized();
}
void GLWindow::DoReleaseSwapchain()
{
	m_swapchainImages.clear();
	m_swapchainFramebuffers.clear();
}

void GLWindow::DoReloadWindow()
{
	ReleaseSwapchain();
	InitWindow(true);
	ReloadSwapchain();
}

void GLWindow::InitCommandBuffers()
{
	auto cmdBuffer = prosper::GLPrimaryCommandBuffer::Create(this->GetContext(), prosper::QueueFamilyType::Universal);
	cmdBuffer->SetDebugName("swapchain_cmd" + std::to_string(0));
	m_commandBuffers = {cmdBuffer, cmdBuffer};
}
