/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_context.hpp"
#include "buffers/gl_buffer.hpp"
#include "image/gl_image.hpp"
#include "image/gl_sampler.hpp"
#include "image/gl_image_view.hpp"
#include "gl_framebuffer.hpp"
#include "gl_render_pass.hpp"
#include "gl_descriptor_set_group.hpp"
#include "gl_fence.hpp"
#include "gl_event.hpp"
#include <buffers/prosper_buffer_create_info.hpp>

using namespace prosper;

#pragma optimize("",off)
std::shared_ptr<GLContext> GLContext::Create(const std::string &appName,bool bEnableValidation)
{
	return std::shared_ptr<GLContext>{new GLContext{appName,bEnableValidation}};
}
GLContext::GLContext(const std::string &appName,bool bEnableValidation)
	: IPrContext{appName,bEnableValidation}
{}
GLContext::~GLContext() {}
bool GLContext::IsImageFormatSupported(
	prosper::Format format,prosper::ImageUsageFlags usageFlags,prosper::ImageType type,
	prosper::ImageTiling tiling
) const
{
	return true; // TODO
}

GLBuffer &GLContext::GetPushConstantBuffer() const {return static_cast<GLBuffer&>(*m_pushConstantBuffer);}

bool GLContext::CheckResult()
{
	if(IsValidationEnabled() == false)
		return true;
	auto err = glGetError();
	if(err == GL_NO_ERROR)
		return true;
	// TODO: ValidationCallback
	return false;
}

void GLContext::ReloadWindow()
{

}

bool GLContext::IsPresentationModeSupported(prosper::PresentModeKHR presentMode) const
{
	return false; // TODO
}
Vendor GLContext::GetPhysicalDeviceVendor() const
{
	return Vendor::AMD; // TODO
}
MemoryRequirements GLContext::GetMemoryRequirements(IImage &img)
{
	return {}; // TODO
}
DeviceSize GLContext::GetBufferAlignment(BufferUsageFlags usageFlags)
{
	return 0; // TODO
}

bool GLContext::SavePipelineCache()
{
	return false; // TODO
}

std::shared_ptr<prosper::IPrimaryCommandBuffer> GLContext::AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType queueFamilyType,uint32_t &universalQueueFamilyIndex)
{
	return nullptr; // TODO
}
std::shared_ptr<prosper::ISecondaryCommandBuffer> GLContext::AllocateSecondaryLevelCommandBuffer(prosper::QueueFamilyType queueFamilyType,uint32_t &universalQueueFamilyIndex)
{
	return nullptr; // TODO
}
void GLContext::SubmitCommandBuffer(prosper::ICommandBuffer &cmd,prosper::QueueFamilyType queueFamilyType,bool shouldBlock,prosper::IFence *fence)
{

}

Result GLContext::WaitForFence(const IFence &fence,uint64_t timeout) const
{
	return Result::ErrorDeviceLost; // TODO
}
Result GLContext::WaitForFences(const std::vector<IFence*> &fences,bool waitAll,uint64_t timeout) const
{
	return Result::ErrorDeviceLost; // TODO
}
void GLContext::DrawFrame(const std::function<void(const std::shared_ptr<prosper::IPrimaryCommandBuffer>&,uint32_t)> &drawFrame)
{

}
bool GLContext::Submit(ICommandBuffer &cmdBuf,bool shouldBlock,IFence *optFence)
{
	return false; // TODO
}

void GLContext::DoKeepResourceAliveUntilPresentationComplete(const std::shared_ptr<void> &resource)
{

}
void GLContext::DoWaitIdle()
{

}
void GLContext::DoFlushSetupCommandBuffer()
{

}
void GLContext::ReloadSwapchain()
{

}

void GLContext::InitAPI(const CreateInfo &createInfo)
{
	InitPushConstantBuffer();
}

void GLContext::InitPushConstantBuffer()
{
	util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = MemoryFeatureFlags::CPUToGPU | MemoryFeatureFlags::WriteOnly;
	bufCreateInfo.size = MAX_COMMON_PUSH_CONSTANT_SIZE;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	m_pushConstantBuffer = CreateBuffer(bufCreateInfo);
}

std::shared_ptr<IBuffer> GLContext::CreateBuffer(const prosper::util::BufferCreateInfo &createInfo,const void *data)
{
	GLuint buf;
	glGenBuffers(1,&buf);

	if(umath::is_flag_set(createInfo.flags,prosper::util::BufferCreateInfo::Flags::DontAllocateMemory) == false)
	{
		auto usage = GL_STREAM_DRAW;
		if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::Stream))
			usage = umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::CopyOnly) ? GL_STREAM_COPY :
				umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::ReadOnly) ? GL_STREAM_READ : GL_STREAM_DRAW;
		else if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::Static))
			usage = umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::CopyOnly) ? GL_STATIC_COPY :
				umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::ReadOnly) ? GL_STATIC_READ : GL_STATIC_DRAW;
		else if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::Dynamic))
			usage = umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::CopyOnly) ? GL_DYNAMIC_COPY :
				umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::ReadOnly) ? GL_DYNAMIC_READ : GL_DYNAMIC_DRAW;
		glNamedBufferData(buf,createInfo.size,data,usage);
	}
	return GLBuffer::Create(*this,createInfo,0,buf);
}
std::shared_ptr<IEvent> GLContext::CreateEvent()
{
	return GLEvent::Create(*this);
}
std::shared_ptr<IFence> GLContext::CreateFence(bool createSignalled)
{
	throw std::runtime_error{"TODO"};
	auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
	return GLFence::Create(*this,fence);
}
std::shared_ptr<ISampler> GLContext::CreateSampler(const prosper::util::SamplerCreateInfo &createInfo)
{
	GLuint sampler;
	glGenSamplers(1,&sampler);
	return GLSampler::Create(*this,createInfo,sampler);
}
std::shared_ptr<IImageView> GLContext::DoCreateImageView(
	const prosper::util::ImageViewCreateInfo &createInfo,IImage &img,Format format,ImageViewType imgViewType,prosper::ImageAspectFlags aspectMask,uint32_t numLayers
)
{
	return GLImageView::Create(*this,img,createInfo,imgViewType,aspectMask);
}
std::shared_ptr<IImage> GLContext::CreateImage(const prosper::util::ImageCreateInfo &createInfo,const uint8_t *data)
{
	GLuint tex;
	glGenTextures(1,&tex);
	// TODO
	return GLImage::Create(*this,createInfo);//,data);
}
std::shared_ptr<IRenderPass> GLContext::CreateRenderPass(const prosper::util::RenderPassCreateInfo &renderPassInfo)
{
	return GLRenderPass::Create(*this,renderPassInfo);
}
std::shared_ptr<IDescriptorSetGroup> GLContext::CreateDescriptorSetGroup(DescriptorSetCreateInfo &descSetInfo)
{
	return GLDescriptorSetGroup::Create(*this,descSetInfo);
}
std::shared_ptr<IFramebuffer> GLContext::CreateFramebuffer(uint32_t width,uint32_t height,uint32_t layers,const std::vector<prosper::IImageView*> &attachments)
{
	GLuint framebuffer;
	glGenFramebuffers(1,&framebuffer);
	std::vector<std::shared_ptr<IImageView>> ptrAttachments {};
	ptrAttachments.reserve(attachments.size());
	for(auto *att : attachments)
		ptrAttachments.push_back(att->shared_from_this());
	auto depth = 1u;
	return GLFramebuffer::Create(*this,ptrAttachments,width,height,depth,layers);
}
#pragma optimize("",on)
