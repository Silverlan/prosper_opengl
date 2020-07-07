/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_context.hpp"
#include "buffers/gl_buffer.hpp"
#include "image/gl_image.hpp"
#include "image/gl_sampler.hpp"
#include "image/gl_image_view.hpp"
#include "buffers/gl_uniform_resizable_buffer.hpp"
#include "buffers/gl_dynamic_resizable_buffer.hpp"
#include "gl_framebuffer.hpp"
#include "gl_render_pass.hpp"
#include "gl_descriptor_set_group.hpp"
#include "gl_fence.hpp"
#include "gl_event.hpp"
#include "gl_api.hpp"
#include "gl_util.hpp"
#include "gl_command_buffer.hpp"
#include "shader/prosper_shader.hpp"
#include "shader/gl_shader_clear.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_glstospv.hpp>
#include <thread>
#include <sharedutils/util_string.h>
#include <buffers/prosper_buffer_create_info.hpp>
#include <iglfw/glfw_window.h>

#pragma optimize("",off)
struct GLShaderStage;
class GLShaderProgram
{
public:
	static std::shared_ptr<GLShaderProgram> Create();
	~GLShaderProgram();
	void AttachShader(GLShaderStage &shaderStage);
	GLuint GetProgramId() const;
	bool Link(std::string &outErr);
private:
	GLShaderProgram(GLuint program);
	GLuint m_program;
};

struct GLShaderStage
	: public prosper::ShaderStageProgram
{
public:
	static std::unique_ptr<GLShaderStage> Compile(prosper::ShaderStage stage,const std::string &glslCode,std::string &outErr);
	~GLShaderStage();
	GLuint GetShaderId() const;
private:
	GLShaderStage(GLuint shader);
	GLuint m_shader;
};

std::unique_ptr<GLShaderStage> GLShaderStage::Compile(prosper::ShaderStage stage,const std::string &glslCode,std::string &outErr)
{
	GLenum glShaderStage;
	switch(stage)
	{
	case prosper::ShaderStage::Compute:
		glShaderStage = GL_COMPUTE_SHADER;
		break;
	case prosper::ShaderStage::Fragment:
		glShaderStage = GL_FRAGMENT_SHADER;
		break;
	case prosper::ShaderStage::Geometry:
		glShaderStage = GL_GEOMETRY_SHADER;
		break;
	case prosper::ShaderStage::TessellationControl:
		glShaderStage = GL_TESS_CONTROL_SHADER;
		break;
	case prosper::ShaderStage::TessellationEvaluation:
		glShaderStage = GL_TESS_EVALUATION_SHADER;
		break;
	case prosper::ShaderStage::Vertex:
		glShaderStage = GL_VERTEX_SHADER;
		break;
	default:
	case prosper::ShaderStage::Unknown:
		outErr = "Unknown shader stage";
		return nullptr;
	}
	auto shader = glCreateShader(glShaderStage);

	auto *pShaderCode = glslCode.c_str();
	glShaderSource(shader,1,&pShaderCode,nullptr);
	glCompileShader(shader);

	GLint compileStatus;
	glGetShaderiv(shader,GL_COMPILE_STATUS,&compileStatus);
	if(compileStatus == GL_FALSE)
	{
		GLsizei len;
		glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&len);
		std::vector<GLchar> vInfoLog;
		vInfoLog.resize(len);
		glGetShaderInfoLog(shader,len,nullptr,vInfoLog.data());

		outErr = std::string{vInfoLog.data()};
		return nullptr;
	}
	return std::unique_ptr<GLShaderStage>{new GLShaderStage{shader}};
}

GLShaderStage::GLShaderStage(GLuint shader)
	: m_shader{shader}
{}
GLShaderStage::~GLShaderStage()
{
	glDeleteShader(m_shader);
}
GLuint GLShaderStage::GetShaderId() const {return m_shader;};

/////////////

std::shared_ptr<GLShaderProgram> GLShaderProgram::Create()
{
	auto programId = glCreateProgram();
	return std::shared_ptr<GLShaderProgram>{new GLShaderProgram{programId}};
}

GLShaderProgram::GLShaderProgram(GLuint program)
	: m_program{program}
{}

GLShaderProgram::~GLShaderProgram()
{
	glDeleteProgram(m_program);
}

void GLShaderProgram::AttachShader(GLShaderStage &shaderStage)
{
	glAttachShader(m_program,shaderStage.GetShaderId());
}
GLuint GLShaderProgram::GetProgramId() const {return m_program;}
bool GLShaderProgram::Link(std::string &outErr)
{
	glLinkProgram(m_program);
	GLint linkStatus;
	glGetProgramiv(m_program,GL_LINK_STATUS,&linkStatus);
	if(linkStatus == GL_FALSE)
	{
		GLsizei len;
		glGetShaderiv(m_program,GL_INFO_LOG_LENGTH,&len);
		std::vector<GLchar> vInfoLog;
		vInfoLog.resize(len);
		glGetProgramInfoLog(m_program,len,nullptr,vInfoLog.data());
		outErr = vInfoLog.data();
		return false;
	}
	return true;
}

////////////////

std::shared_ptr<prosper::GLContext> prosper::GLContext::Create(const std::string &appName,bool bEnableValidation)
{
	return std::shared_ptr<prosper::GLContext>{new GLContext{appName,bEnableValidation}};
}
prosper::GLContext::GLContext(const std::string &appName,bool bEnableValidation)
	: IPrContext{appName,bEnableValidation}
{}
prosper::GLContext::~GLContext()
{
	m_pipelines.clear();
}
bool prosper::GLContext::IsImageFormatSupported(
	prosper::Format format,prosper::ImageUsageFlags usageFlags,prosper::ImageType type,
	prosper::ImageTiling tiling
) const
{
	return true; // TODO
}
uint32_t prosper::GLContext::GetUniversalQueueFamilyIndex() const {return 0;}
prosper::util::Limits prosper::GLContext::GetPhysicalDeviceLimits() const
{
	util::Limits limits {};
	limits.maxSamplerAnisotropy = 16.f; // We can't query it, but 16 is widely supported: https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxSamplerAnisotropy&platform=windows
	limits.maxStorageBufferRange = 134'217'728; // Maximum storage buffer range supported by some intel GPUs: https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxStorageBufferRange&platform=windows
	limits.maxImageArrayLayers = 2'048; // Max widely-supported image array layers: https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxImageArrayLayers&platform=windows
	limits.maxSurfaceImageCount = std::numeric_limits<uint32_t>::max();
	return limits;
}
std::optional<prosper::util::PhysicalDeviceImageFormatProperties> prosper::GLContext::GetPhysicalDeviceImageFormatProperties(const ImageFormatPropertiesQuery &query)
{
	GLint samples = 1;
	glGetInternalformativ(GL_RENDERBUFFER,util::to_opengl_image_format(query.format),GL_SAMPLES,1,&samples);
	prosper::util::PhysicalDeviceImageFormatProperties props {};
	props.sampleCount = static_cast<prosper::SampleCountFlags>(samples);
	return props;
}
bool prosper::GLContext::CheckFramebufferStatus(IFramebuffer &fb) const
{
	auto status = glCheckNamedFramebufferStatus(static_cast<GLFramebuffer&>(fb).GetGLFramebuffer(),GL_FRAMEBUFFER);
	std::string statusMsg;
	switch(status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		statusMsg = "Complete";
		return true;
	case GL_FRAMEBUFFER_UNDEFINED:
		statusMsg = "Undefined";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		statusMsg = "Incomplete: Attachment";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		statusMsg = "Incomplete: Missing attachment";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		statusMsg = "Incomplete: Draw buffer";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		statusMsg = "Incomplete: Read buffer";
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		statusMsg = "Unsupported";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		statusMsg = "Incomplete: Multisample";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		statusMsg = "Incomplete: Layer targets";
		break;
	}
	const_cast<GLContext*>(this)->ValidationCallback(DebugMessageSeverityFlags::ErrorBit,"Framebuffer status: " +statusMsg);
	return false;
}

static std::string error_to_string(GLenum err)
{
	switch(err)
	{
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_CONTEXT_LOST:
		return "GL_CONTEXT_LOST";
	default:
		return std::to_string(err);
	}
}

prosper::GLBuffer &prosper::GLContext::GetPushConstantBuffer() const {return dynamic_cast<GLBuffer&>(*m_pushConstantBuffer);}

std::optional<GLuint> prosper::GLContext::GetPipelineProgram(PipelineID pipelineId) const
{
	return (pipelineId < m_pipelines.size()) ? m_pipelines.at(pipelineId)->GetProgramId() : std::optional<GLuint>{};
}

bool prosper::GLContext::CheckResult()
{
	if(IsValidationEnabled() == false)
		return true;
	auto err = glGetError();
	if(err == GL_NO_ERROR)
		return true;
	return ValidationCallback(prosper::DebugMessageSeverityFlags::ErrorBit,error_to_string(err));
}

void prosper::GLContext::ReloadWindow()
{
	WaitIdle();
	InitWindow();
}

prosper::IFramebuffer *prosper::GLContext::GetSwapchainFramebuffer(uint32_t idx)
{
	return (idx < m_swapchainFramebuffers.size()) ? m_swapchainFramebuffers.at(idx).get() : nullptr;
}

void prosper::GLContext::InitCommandBuffers()
{
	auto cmdBuffer = prosper::GLPrimaryCommandBuffer::Create(*this,prosper::QueueFamilyType::Universal);
	cmdBuffer->SetDebugName("swapchain_cmd" +std::to_string(0));
	m_commandBuffers = {cmdBuffer};
}

void prosper::GLContext::InitWindow()
{
	auto oldSize = (m_glfwWindow != nullptr) ? m_glfwWindow->GetSize() : Vector2i();
	auto &appName = GetAppName();
	/* Create a window */
	//m_windowPtr = Anvil::WindowFactory::create_window(platform,appName,width,height,true,std::bind(&Context::DrawFrame,this));

	// TODO: Clean this up
	m_glfwWindow = nullptr;
	GLFW::poll_events();
	m_windowCreationInfo->api = GLFW::WindowCreationInfo::API::OpenGL;
	m_glfwWindow = GLFW::Window::Create(*m_windowCreationInfo); // TODO: Release

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
	m_windowCreationInfo->width = actualWindowSize.x;
	m_windowCreationInfo->height = actualWindowSize.y;

	m_glfwWindow->SetResizeCallback([](GLFW::Window &window,Vector2i size) {
		std::cout<<"Resizing..."<<std::endl; // TODO
	});
	glfwMakeContextCurrent(const_cast<GLFWwindow*>(m_glfwWindow->GetGLFWWindow()));

	OnWindowInitialized();
	if(m_glfwWindow != nullptr && umath::is_flag_set(m_stateFlags,StateFlags::Initialized) == true)
	{
		auto newSize = m_glfwWindow->GetSize();
		if(newSize != oldSize)
			OnResolutionChanged(newSize.x,newSize.y);
	}
}

bool prosper::GLContext::IsPresentationModeSupported(prosper::PresentModeKHR presentMode) const
{
	// OpenGL doesn't know presentation modes, so we don't really care.
	return true;
}

std::unique_ptr<prosper::ShaderModule> prosper::GLContext::CreateShaderModuleFromStageData(
	const std::shared_ptr<ShaderStageProgram> &shaderStageProgram,
	prosper::ShaderStage stage,
	const std::string &entrypointName
)
{
	return nullptr;
}
std::shared_ptr<prosper::ShaderStageProgram> prosper::GLContext::CompileShader(prosper::ShaderStage stage,const std::string &shaderPath,std::string &outInfoLog,std::string &outDebugInfoLog,bool reload)
{
	auto shaderCode = prosper::load_glsl(*this,stage,shaderPath,&outInfoLog,&outDebugInfoLog);
	if(shaderCode.has_value() == false)
		return nullptr;
	return GLShaderStage::Compile(stage,*shaderCode,outInfoLog);
}

/////////////

prosper::Vendor prosper::GLContext::GetPhysicalDeviceVendor() const
{
	std::string vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
	if(ustring::match(vendor,"*intel*"))
		return Vendor::Intel;
	else if(ustring::match(vendor,"*AMD*"))
		return Vendor::AMD;
	else if(ustring::match(vendor,"*NVIDIA*"))
		return Vendor::Nvidia;
	return Vendor::Unknown;
}
prosper::MemoryRequirements prosper::GLContext::GetMemoryRequirements(IImage &img)
{
	MemoryRequirements memReq {};
	auto format = img.GetFormat();
	auto compressedFormat = util::is_compressed_format(format);
	auto numMipmaps = img.GetMipmapCount();
	for(auto i=decltype(numMipmaps){0u};i<numMipmaps;++i)
	{
		GLint size = 0;
		if(compressedFormat)
			glGetTexLevelParameteriv(GL_TEXTURE_2D,i,GL_TEXTURE_COMPRESSED_IMAGE_SIZE,&size);
		else
		{
			auto width = img.GetWidth(i);
			auto height = img.GetHeight(i);
			size += width *height *util::get_byte_size(format);
		}
		memReq.size += size;
	}
	return memReq;
}

uint64_t prosper::GLContext::ClampDeviceMemorySize(uint64_t size,float percentageOfGPUMemory,MemoryFeatureFlags featureFlags) const
{
	// TODO
	return size;
}
prosper::DeviceSize prosper::GLContext::CalcBufferAlignment(BufferUsageFlags usageFlags)
{
	return 0; // TODO
}

void prosper::GLContext::GetGLSLDefinitions(GLSLDefinitions &outDef) const
{
	outDef.layoutIdBuffer = "binding = (setIndex *10 +bindingIndex +1)";
	outDef.layoutIdImage = "binding = (setIndex *10 +bindingIndex +1)";
	outDef.layoutPushConstants = "std140, binding = 0"; // Index 0 is reserved for push constants
	outDef.vertexIndex = "gl_VertexID";
}

bool prosper::GLContext::SavePipelineCache()
{
	return false; // TODO
}

std::shared_ptr<prosper::IPrimaryCommandBuffer> prosper::GLContext::AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType queueFamilyType,uint32_t &universalQueueFamilyIndex)
{
	return GLPrimaryCommandBuffer::Create(*this,queueFamilyType);
}
std::shared_ptr<prosper::ISecondaryCommandBuffer> prosper::GLContext::AllocateSecondaryLevelCommandBuffer(prosper::QueueFamilyType queueFamilyType,uint32_t &universalQueueFamilyIndex)
{
	return GLSecondaryCommandBuffer::Create(*this,queueFamilyType);
}
void prosper::GLContext::SubmitCommandBuffer(prosper::ICommandBuffer &cmd,prosper::QueueFamilyType queueFamilyType,bool shouldBlock,prosper::IFence *fence)
{

}

std::optional<prosper::PipelineID> prosper::GLContext::AddPipeline(
	const prosper::ComputePipelineCreateInfo &createInfo,
	prosper::ShaderStageData &stage,PipelineID basePipelineId
)
{
	auto program = GLShaderProgram::Create();
	program->AttachShader(static_cast<GLShaderStage&>(*stage.program));
	if(CheckResult() == false)
		return {};
	std::string err;
	if(program->Link(err) == false)
	{
		ValidationCallback(prosper::DebugMessageSeverityFlags::ErrorBit,err);
		return false;
	}
	m_pipelines.push_back(program);
	return m_pipelines.size() -1;
}
std::optional<prosper::PipelineID> prosper::GLContext::AddPipeline(
	const prosper::GraphicsPipelineCreateInfo &createInfo,
	IRenderPass &rp,
	prosper::ShaderStageData *shaderStageFs,
	prosper::ShaderStageData *shaderStageVs,
	prosper::ShaderStageData *shaderStageGs,
	prosper::ShaderStageData *shaderStageTc,
	prosper::ShaderStageData *shaderStageTe,
	SubPassID subPassId,
	PipelineID basePipelineId
)
{
	auto program = GLShaderProgram::Create();
	for(auto *shaderStage : std::initializer_list<prosper::ShaderStageData*>{shaderStageFs,shaderStageVs,shaderStageGs,shaderStageTc,shaderStageTe})
	{
		if(shaderStage == nullptr)
			continue;
		program->AttachShader(static_cast<GLShaderStage&>(*shaderStage->program));
		if(CheckResult() == false)
			return {};
	}
	if(CheckResult() == false)
		return {};
	std::string err;
	if(program->Link(err) == false)
	{
		ValidationCallback(prosper::DebugMessageSeverityFlags::ErrorBit,err);
		return false;
	}
	if(m_freePipelineIndices.empty() == false)
	{
		auto idx = m_freePipelineIndices.front();
		m_freePipelineIndices.pop();
		m_pipelines.at(idx) = program;
		return idx;
	}
	m_pipelines.push_back(program);
	return m_pipelines.size() -1;
}

bool prosper::GLContext::ClearPipeline(bool graphicsShader,PipelineID pipelineId)
{
	m_pipelines.at(pipelineId) = nullptr;
	m_freePipelineIndices.push(pipelineId);
	return true;
}

prosper::Result prosper::GLContext::WaitForFence(const IFence &fence,uint64_t timeout) const
{

	return Result::ErrorDeviceLost; // TODO
}
prosper::Result prosper::GLContext::WaitForFences(const std::vector<IFence*> &fences,bool waitAll,uint64_t timeout) const
{
	return Result::ErrorDeviceLost; // TODO
}
std::shared_ptr<prosper::IQueryPool> prosper::GLContext::CreateQueryPool(QueryType queryType,uint32_t maxConcurrentQueries)
{
	return nullptr; // TODO
}
std::shared_ptr<prosper::IQueryPool> prosper::GLContext::CreateQueryPool(QueryPipelineStatisticFlags statsFlags,uint32_t maxConcurrentQueries)
{
	return nullptr; // TODO
}
bool prosper::GLContext::QueryResult(const TimestampQuery &query,std::chrono::nanoseconds &outTimestampValue) const
{
	return false; // TODO
}
bool prosper::GLContext::QueryResult(const PipelineStatisticsQuery &query,PipelineStatistics &outStatistics) const
{
	return false; // TODO
}
bool prosper::GLContext::QueryResult(const Query &query,uint32_t &r) const
{
	return false; // TODO
}
bool prosper::GLContext::QueryResult(const Query &query,uint64_t &r) const
{
	return false; // TODO
}
void prosper::GLContext::DrawFrame(const std::function<void(const std::shared_ptr<prosper::IPrimaryCommandBuffer>&,uint32_t)> &drawFrame)
{
	// TODO: Wait for fences?
	ClearKeepAliveResources();

	auto &cmdBuffer = m_commandBuffers.at(m_n_swapchain_image);
	// TODO: Start recording?
	cmdBuffer->StartRecording(false,true);
	umath::set_flag(m_stateFlags,StateFlags::IsRecording);
	umath::set_flag(m_stateFlags,StateFlags::Idle,false);
	while(m_scheduledBufferUpdates.empty() == false)
	{
		auto &f = m_scheduledBufferUpdates.front();
		f(*cmdBuffer);
		m_scheduledBufferUpdates.pop();
	}
	drawFrame(GetDrawCommandBuffer(),m_n_swapchain_image);

	/* Close the recording process */
	umath::set_flag(m_stateFlags,StateFlags::IsRecording,false);
	cmdBuffer->StopRecording();
	// TODO: Submit command buffer?
	m_glfwWindow->SwapBuffers();
}
bool prosper::GLContext::Submit(ICommandBuffer &cmdBuf,bool shouldBlock,IFence *optFence)
{
	return false; // TODO
}
void prosper::GLContext::Initialize(const CreateInfo &createInfo)
{
	IPrContext::Initialize(createInfo);
	m_hShaderClear = m_shaderManager->RegisterShader("clear_image",[](prosper::IPrContext &context,const std::string &identifier) {return new prosper::ShaderClear(context,identifier);});
}
prosper::ShaderClear *prosper::GLContext::GetClearShader() const {return static_cast<prosper::ShaderClear*>(m_hShaderClear.get());}

void prosper::GLContext::DoKeepResourceAliveUntilPresentationComplete(const std::shared_ptr<void> &resource) {}
void prosper::GLContext::DoWaitIdle() {glFinish();}
void prosper::GLContext::DoFlushSetupCommandBuffer()
{

}
void prosper::GLContext::ReloadSwapchain()
{

}

void prosper::GLContext::InitAPI(const CreateInfo &createInfo)
{
	InitWindow();
	InitCommandBuffers();
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout<<"Failed to initialize GLAD"<<std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		exit(EXIT_FAILURE);
	}
	m_shaderManager = std::make_unique<ShaderManager>(*this);
	InitPushConstantBuffer();
	InitTemporaryBuffer();
	InitSwapchainImages();
	CheckResult();
}

void prosper::GLContext::InitSwapchainImages()
{
	util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.width = GetWindowWidth();
	imgCreateInfo.height = GetWindowHeight();
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.layers = 1;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	imgCreateInfo.tiling = prosper::ImageTiling::Optimal;
	imgCreateInfo.usage = prosper::ImageUsageFlags::TransferDstBit;
	imgCreateInfo.flags = util::ImageCreateInfo::Flags::DontAllocateMemory;
	auto img = std::shared_ptr<GLImage>{new GLImage{*this,imgCreateInfo,0}};
	m_swapchainImages = {img};

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.baseLayer = 0;
	imgViewCreateInfo.baseMipmap = 0;
	imgViewCreateInfo.mipmapLevels = 1;
	imgViewCreateInfo.levelCount = 1;
	imgViewCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	auto imgView = GLImageView::Create(*this,*img,imgViewCreateInfo,prosper::ImageViewType::e2D,prosper::ImageAspectFlags::ColorBit);
	auto framebuffer = std::shared_ptr<GLFramebuffer>{new GLFramebuffer{*this,{imgView},imgCreateInfo.width,imgCreateInfo.height,1,1,0}};
	m_swapchainFramebuffers = {framebuffer};
}

void prosper::GLContext::InitPushConstantBuffer()
{
	util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = MemoryFeatureFlags::CPUToGPU | MemoryFeatureFlags::WriteOnly;
	bufCreateInfo.size = MAX_COMMON_PUSH_CONSTANT_SIZE;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	m_pushConstantBuffer = CreateBuffer(bufCreateInfo);
}

std::shared_ptr<prosper::IBuffer> prosper::GLContext::CreateBuffer(const prosper::util::BufferCreateInfo &createInfo,const void *data)
{
	GLuint buf;
	glCreateBuffers(1,&buf);

	if(umath::is_flag_set(createInfo.flags,prosper::util::BufferCreateInfo::Flags::DontAllocateMemory) == false)
	{
		// TODO: Remove Steam, Dynamic and Static flags?
#if 0
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
#endif
		GLbitfield flags = 0;
		if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::ReadOnly) == false)
			flags |= GL_MAP_WRITE_BIT;
		if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::HostCoherent))
			flags |= GL_MAP_COHERENT_BIT;
		if(umath::is_flag_set(createInfo.flags,util::BufferCreateInfo::Flags::Persistent))
			flags |= GL_MAP_PERSISTENT_BIT;
		if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::HostCached) && umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::DeviceLocal) == false)
			flags |= GL_CLIENT_STORAGE_BIT;
		if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::HostAccessable))
			flags |= GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT;
		glNamedBufferStorage(buf,createInfo.size,data,flags);
	}
	return GLBuffer::Create(*this,createInfo,0,buf);
}

std::shared_ptr<prosper::IUniformResizableBuffer> prosper::GLContext::DoCreateUniformResizableBuffer(
	const prosper::util::BufferCreateInfo &createInfo,uint64_t bufferInstanceSize,
	uint64_t maxTotalSize,const void *data,
	prosper::DeviceSize bufferBaseSize,uint32_t alignment
)
{
	auto buf = CreateBuffer(createInfo,data);
	if(buf == nullptr)
		return nullptr;
	auto r = std::shared_ptr<GLUniformResizableBuffer>(new GLUniformResizableBuffer{*this,*buf,bufferInstanceSize,bufferBaseSize,maxTotalSize,alignment});
	r->Initialize();
	return r;
}
std::shared_ptr<prosper::IDynamicResizableBuffer> prosper::GLContext::CreateDynamicResizableBuffer(
	prosper::util::BufferCreateInfo createInfo,
	uint64_t maxTotalSize,float clampSizeToAvailableGPUMemoryPercentage,const void *data
)
{
	createInfo.size = ClampDeviceMemorySize(createInfo.size,clampSizeToAvailableGPUMemoryPercentage,createInfo.memoryFeatures);
	maxTotalSize = ClampDeviceMemorySize(maxTotalSize,clampSizeToAvailableGPUMemoryPercentage,createInfo.memoryFeatures);
	auto buf = CreateBuffer(createInfo,data);
	if(buf == nullptr)
		return nullptr;
	auto r = std::shared_ptr<GLDynamicResizableBuffer>(new GLDynamicResizableBuffer{*this,*buf,createInfo,maxTotalSize});
	r->Initialize();
	return r;
}
std::shared_ptr<prosper::IEvent> prosper::GLContext::CreateEvent()
{
	return GLEvent::Create(*this);
}
std::shared_ptr<prosper::IFence> prosper::GLContext::CreateFence(bool createSignalled)
{
	throw std::runtime_error{"TODO"};
	auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
	return GLFence::Create(*this,fence);
}
std::shared_ptr<prosper::ISampler> prosper::GLContext::CreateSampler(const prosper::util::SamplerCreateInfo &createInfo)
{
	GLuint sampler;
	glCreateSamplers(1,&sampler);
	return GLSampler::Create(*this,createInfo,sampler);
}
std::shared_ptr<prosper::IImageView> prosper::GLContext::DoCreateImageView(
	const prosper::util::ImageViewCreateInfo &createInfo,IImage &img,Format format,ImageViewType imgViewType,prosper::ImageAspectFlags aspectMask,uint32_t numLayers
)
{
	return GLImageView::Create(*this,img,createInfo,imgViewType,aspectMask);
}
std::shared_ptr<prosper::IImage> prosper::GLContext::CreateImage(const prosper::util::ImageCreateInfo &createInfo,const uint8_t *data)
{
	return GLImage::Create(*this,createInfo);
}
std::shared_ptr<prosper::IRenderPass> prosper::GLContext::CreateRenderPass(const prosper::util::RenderPassCreateInfo &renderPassInfo)
{
	return GLRenderPass::Create(*this,renderPassInfo);
}
std::shared_ptr<prosper::IDescriptorSetGroup> prosper::GLContext::CreateDescriptorSetGroup(DescriptorSetCreateInfo &descSetInfo)
{
	return GLDescriptorSetGroup::Create(*this,descSetInfo);
}
std::shared_ptr<prosper::IFramebuffer> prosper::GLContext::CreateFramebuffer(uint32_t width,uint32_t height,uint32_t layers,const std::vector<prosper::IImageView*> &attachments)
{
	std::vector<std::shared_ptr<IImageView>> ptrAttachments {};
	ptrAttachments.reserve(attachments.size());
	for(auto *att : attachments)
		ptrAttachments.push_back(att->shared_from_this());

	auto depth = 1u;
	return GLFramebuffer::Create(*this,ptrAttachments,width,height,depth,layers);
}
#pragma optimize("",on)
