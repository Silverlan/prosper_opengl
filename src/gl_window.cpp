/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_context.hpp"
#include "gl_window.hpp"
#include "image/gl_image.hpp"
#include "image/gl_image_view.hpp"
#include "gl_framebuffer.hpp"
#include "gl_api.hpp"
#include <thread>

using namespace prosper;

std::shared_ptr<GLWindow> GLWindow::Create(const WindowSettings &windowCreationInfo,prosper::IPrContext &context)
{
	return std::shared_ptr<GLWindow>{new GLWindow{context,windowCreationInfo}};
}

void GLWindow::InitWindow()
{
	/* Create a window */
	//m_windowPtr = Anvil::WindowFactory::create_window(platform,appName,width,height,true,std::bind(&Context::DrawFrame,this));

	// TODO: Clean this up
	m_glfwWindow = nullptr;
	GLFW::poll_events();
	auto settings = m_settings;
	if(!settings.windowedMode && (settings.monitor.has_value() == false || settings.monitor->GetGLFWMonitor() == nullptr))
		settings.monitor = GLFW::get_primary_monitor();
	settings.api = GLFW::WindowCreationInfo::API::OpenGL;
	if(GetContext().IsValidationEnabled())
		settings.flags |= GLFW::WindowCreationInfo::Flags::DebugContext;
	m_glfwWindow = GLFW::Window::Create(settings); // TODO: Release

	const char *errDesc;
	auto err = glfwGetError(&errDesc);
	if(err != GLFW_NO_ERROR)
	{
		std::cout<<"Error retrieving GLFW window handle: "<<errDesc<<std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		exit(EXIT_FAILURE);
	}

	// GLFW does not guarantee to actually use the size which was specified,
	// in some cases it may change, so we have to retrieve it again here
	auto actualWindowSize = m_glfwWindow->GetSize();
	m_settings.width = actualWindowSize.x;
	m_settings.height = actualWindowSize.y;

	m_glfwWindow->SetResizeCallback([](GLFW::Window &window,Vector2i size) {
		std::cout<<"Resizing..."<<std::endl; // TODO
	});
	glfwMakeContextCurrent(const_cast<GLFWwindow*>(m_glfwWindow->GetGLFWWindow()));

	OnWindowInitialized();
}
void GLWindow::ReleaseWindow()
{
	ReleaseSwapchain();
	m_glfwWindow = nullptr;
}
void GLWindow::InitSwapchain()
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
	auto img = std::shared_ptr<GLImage>{new GLImage{GetContext(),imgCreateInfo,0,GL_RGBA}};
	m_swapchainImages = {img,img};

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.baseLayer = 0;
	imgViewCreateInfo.baseMipmap = 0;
	imgViewCreateInfo.mipmapLevels = 1;
	imgViewCreateInfo.levelCount = 1;
	imgViewCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	auto imgView = GLImageView::Create(GetContext(),*img,imgViewCreateInfo,prosper::ImageViewType::e2D,prosper::ImageAspectFlags::ColorBit);
	auto framebuffer = std::shared_ptr<GLFramebuffer>{new GLFramebuffer{GetContext(),{imgView},imgCreateInfo.width,imgCreateInfo.height,1,1,0}};
	m_swapchainFramebuffers = {framebuffer,framebuffer};
}
void GLWindow::ReleaseSwapchain()
{
	m_swapchainImages.clear();
	m_swapchainFramebuffers.clear();
}
