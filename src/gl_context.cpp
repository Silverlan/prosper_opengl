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
#include "buffers/gl_render_buffer.hpp"
#include "shader/gl_shader_post_processing.hpp"
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
#include "shader/gl_shader_blit.hpp"
#include "shader/gl_shader_flip_y.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_glsl.hpp>
#include <thread>
#include <sharedutils/util_string.h>
#include <buffers/prosper_buffer_create_info.hpp>
#include <iglfw/glfw_window.h>
#include <fsys/filesystem.h>

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
		GLsizei len = 0;
		glGetProgramiv(m_program,GL_INFO_LOG_LENGTH,&len);
		std::vector<GLchar> vInfoLog;
		vInfoLog.resize(len);
		len = 0;
		glGetProgramInfoLog(m_program,len,nullptr,vInfoLog.data());
		if(len > 0)
			outErr = vInfoLog.data();
		else
			outErr = "";
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
	return (pipelineId < m_pipelines.size() && m_pipelines.at(pipelineId).program) ? m_pipelines.at(pipelineId).program->GetProgramId() : std::optional<GLuint>{};
}

bool prosper::GLContext::CheckResult()
{
	if(IsValidationEnabled() == false)
		return true;
	auto err = glGetError();
	if(err == GL_NO_ERROR)
		return true;
	ValidationCallback(prosper::DebugMessageSeverityFlags::ErrorBit,error_to_string(err));
	return false;
}

void prosper::GLContext::ReloadWindow()
{
	WaitIdle();

	auto oldSize = (m_glfwWindow != nullptr) ? m_glfwWindow->GetSize() : Vector2i();
	auto w = m_windowCreationInfo->width;
	auto h = m_windowCreationInfo->height;
	m_glfwWindow->SetSize(Vector2i{w,h});
	m_glfwWindow->UpdateWindow(*m_windowCreationInfo);

	for(auto &img : m_swapchainImages)
	{
		auto &createInfo = const_cast<prosper::util::ImageCreateInfo&>(img->GetCreateInfo());
		createInfo.width = w;
		createInfo.height = h;
	}
	for(auto &fb : m_swapchainFramebuffers)
		static_cast<GLFramebuffer&>(*fb).UpateSize(w,h);

	OnResolutionChanged(w,h);
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
	auto &appName = GetAppName();
	/* Create a window */
	//m_windowPtr = Anvil::WindowFactory::create_window(platform,appName,width,height,true,std::bind(&Context::DrawFrame,this));

	// TODO: Clean this up
	m_glfwWindow = nullptr;
	GLFW::poll_events();
	m_windowCreationInfo->api = GLFW::WindowCreationInfo::API::OpenGL;
	if(IsValidationEnabled())
		m_windowCreationInfo->flags |= GLFW::WindowCreationInfo::Flags::DebugContext;
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
	auto shaderCode = prosper::glsl::load_glsl(*this,stage,shaderPath,&outInfoLog,&outDebugInfoLog);
	if(shaderCode.has_value() == false)
		return nullptr;
	return GLShaderStage::Compile(stage,*shaderCode,outInfoLog);
}
bool prosper::GLContext::InitializeShaderSources(prosper::Shader &shader,bool bReload,std::string &outInfoLog,std::string &outDebugInfoLog,prosper::ShaderStage &outErrStage) const
{
	outErrStage = prosper::ShaderStage::Unknown;
	auto &stages = shader.GetStages();
	std::vector<std::string> glslCodePerStage {};
	std::vector<prosper::ShaderStage> glslCodeStages {};
	glslCodePerStage.reserve(stages.size());
	glslCodeStages.reserve(stages.size());
	for(auto i=decltype(stages.size()){0};i<stages.size();++i)
	{
		auto &stage = stages.at(i);
		if(stage == nullptr || stage->path.empty())
			continue;
		stage->stage = static_cast<prosper::ShaderStage>(i);
		auto glslCode = prosper::glsl::load_glsl(const_cast<GLContext&>(*this),stage->stage,stage->path,&outInfoLog,&outDebugInfoLog);
		if(glslCode.has_value() == false)
		{
			outErrStage = stage->stage;
			return false;
		}
		glslCodePerStage.emplace_back(std::move(*glslCode));
		glslCodeStages.push_back(stage->stage);
	}
	if(util::convert_glsl_set_bindings_to_opengl_binding_points(glslCodePerStage,outInfoLog) == false)
		return false;
	auto &logCallback = shader.GetLogCallback();
	for(auto i=decltype(glslCodePerStage.size()){0u};i<glslCodePerStage.size();++i)
	{
		auto stage = glslCodeStages.at(i);
		std::shared_ptr<GLShaderStage> shaderStageProgram = GLShaderStage::Compile(stage,glslCodePerStage.at(i),outInfoLog);
		if(shaderStageProgram == nullptr)
		{
			outErrStage = stage;
			return false;
		}
		stages.at(umath::to_integral(stage))->program = shaderStageProgram;
	}
#if 0
	// Output final shader code
	// For debugging purposes only
	if(shader.GetIdentifier() == "pbr")
	{
		{
			auto stage = prosper::ShaderStage::Fragment;
			auto it = std::find_if(glslCodeStages.begin(),glslCodeStages.end(),[stage](const prosper::ShaderStage &stageOther) {return stageOther == stage;});
			std::string postfix;
			if(stage == prosper::ShaderStage::Fragment)
				postfix = "_frag";
			else if(stage == prosper::ShaderStage::Vertex)
				postfix = "_vert";
			auto f = FileManager::OpenFile<VFilePtrReal>(("shader_error" +postfix +".txt").c_str(),"wb");
			if(f)
			{
				f->WriteString(glslCodePerStage.at(it -glslCodeStages.begin()));
				f = nullptr;
			}
		}
	}
#endif
	return true;
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
			glGetTextureLevelParameteriv(static_cast<GLImage&>(img).GetGLImage(),i,GL_TEXTURE_COMPRESSED_IMAGE_SIZE,&size);
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
	GLint bufferOffsetAlignment = 0;
	if(umath::is_flag_set(usageFlags,BufferUsageFlags::UniformBufferBit))
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,&bufferOffsetAlignment);
	return bufferOffsetAlignment;
}

void prosper::GLContext::GetGLSLDefinitions(glsl::Definitions &outDef) const
{
	outDef.layoutId = "binding = (setIndex *10 +bindingIndex +1)";
	outDef.layoutPushConstants = "std140, binding = 0"; // Index 0 is reserved for push constants
	outDef.vertexIndex = "gl_VertexID";
	outDef.instanceIndex = "gl_InstanceID";
	// Matrix for converting depth range and y-axis inversion
	outDef.apiCoordTransform = "mat4(1,0,0,0,0,-1,0,0,0,0,2,0,0,0,-1,1) *T"; // Matrix is inverse of mat4(1,0,0,0,0,-1,0,0,0,0,0.5,0,0,0,0.5,1)
	outDef.apiScreenSpaceTransform = "mat4(1,0,0,0,0,-1,0,0,0,0,1,0,0,0,1,1) *T";
	outDef.apiDepthTransform = "mat4(1,0,0,0,0,1,0,0,0,0,2,0,0,0,-1,1) *T"; // Matrix is inverse of mat4(1,0,0,0,0,1,0,0,0,0,0.5,0,0,0,0.5,1)
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
	auto *glFence = static_cast<prosper::GLFence*>(fence);
	if(glFence)
	{
		if(shouldBlock)
			glFence->Wait();
		glFence->Reset();
	}
}

prosper::PipelineID prosper::GLContext::AddPipeline(prosper::Shader &shader,PipelineID shaderPipelineId,std::shared_ptr<GLShaderProgram> program)
{
	PipelineID pipelineId;
	if(m_freePipelineIndices.empty() == false)
	{
		auto idx = m_freePipelineIndices.front();
		m_freePipelineIndices.pop();
		m_pipelines.at(idx).program = program;
		pipelineId = idx;
	}
	else
	{
		m_pipelines.push_back({});
		m_pipelines.back().program = program;
		pipelineId = m_pipelines.size() -1;
	}
	InitShaderPipeline(shader,pipelineId,shaderPipelineId);
	return pipelineId;
}
std::optional<prosper::PipelineID> prosper::GLContext::AddPipeline(
	prosper::Shader &shader,PipelineID shaderPipelineId,const prosper::ComputePipelineCreateInfo &createInfo,
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
	return AddPipeline(shader,shaderPipelineId,program);
}
std::optional<prosper::PipelineID> prosper::GLContext::AddPipeline(
	prosper::Shader &shader,PipelineID shaderPipelineId,
	const prosper::GraphicsPipelineCreateInfo &createInfo,IRenderPass &rp,
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
		return {};
	}
	return AddPipeline(shader,shaderPipelineId,program);
}

std::optional<uint32_t> prosper::GLContext::ShaderPipelineDescSetBindingIndexToBindingPoint(PipelineID pipelineId,uint32_t setIdx,uint32_t bindingIdx) const
{
	if(pipelineId >= m_pipelines.size())
		return {};
	auto &pipelineData = m_pipelines.at(pipelineId);
	if(setIdx >= pipelineData.descriptorSetBindingsToBindingPoints.size())
		return {};
	auto &bindingPoints = pipelineData.descriptorSetBindingsToBindingPoints.at(setIdx);
	return (bindingIdx < bindingPoints.size()) ? bindingPoints.at(bindingIdx) : std::optional<uint32_t>{};
}

void prosper::GLContext::InitShaderPipeline(prosper::Shader &shader,PipelineID pipelineId,PipelineID shaderPipelineId)
{
	auto &pipelineInfo = *shader.GetPipelineInfo(shaderPipelineId);
	auto &pipelineData = m_pipelines.at(pipelineId);
	uint32_t bufferBindingPoint = 1; // 0 is reserved for push constants
	uint32_t imageBindingPoint = 0;
	pipelineData.descriptorSetBindingsToBindingPoints.reserve(pipelineInfo.descSetInfos.size());
	for(auto &dsCreateInfo : pipelineInfo.descSetInfos)
	{
		pipelineData.descriptorSetBindingsToBindingPoints.push_back({});
		auto &dsBindingsToBindingPoints = pipelineData.descriptorSetBindingsToBindingPoints.back();
		auto numBindings = dsCreateInfo->GetBindingCount();
		dsBindingsToBindingPoints.resize(numBindings,std::numeric_limits<uint32_t>::max());
		for(auto i=decltype(numBindings){0u};i<numBindings;++i)
		{
			prosper::DescriptorType descType;
			uint32_t arraySize = 0;
			if(dsCreateInfo->GetBindingPropertiesByBindingIndex(
					i,&descType,&arraySize
				) == false)
				continue;
			arraySize = umath::max(arraySize,static_cast<uint32_t>(1));
			switch(descType)
			{
			case prosper::DescriptorType::Sampler:
			case prosper::DescriptorType::CombinedImageSampler:
			case prosper::DescriptorType::SampledImage:
			case prosper::DescriptorType::StorageImage:
				dsBindingsToBindingPoints.at(i) = imageBindingPoint;
				imageBindingPoint += arraySize;
				break;
			default:
				dsBindingsToBindingPoints.at(i) = bufferBindingPoint;
				bufferBindingPoint += arraySize;
				break;
			}
		}
	}
}

bool prosper::GLContext::ClearPipeline(bool graphicsShader,PipelineID pipelineId)
{
	m_pipelines.at(pipelineId) = {};
	m_freePipelineIndices.push(pipelineId);
	return true;
}
uint32_t prosper::GLContext::GetLastAcquiredSwapchainImageIndex() const {return 0;}

void prosper::GLContext::Flush() {glFlush();}
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
	m_hShaderBlit = m_shaderManager->RegisterShader("blit_image",[](prosper::IPrContext &context,const std::string &identifier) {return new prosper::ShaderBlit(context,identifier);});
	m_hShaderFlipY = m_shaderManager->RegisterShader("flip_y",[](prosper::IPrContext &context,const std::string &identifier) {return new prosper::ShaderFlipY(context,identifier);});
}
prosper::ShaderClear *prosper::GLContext::GetClearShader() const {return static_cast<prosper::ShaderClear*>(m_hShaderClear.get());}
prosper::ShaderBlit *prosper::GLContext::GetBlitShader() const {return static_cast<prosper::ShaderBlit*>(m_hShaderBlit.get());}
prosper::ShaderFlipY *prosper::GLContext::GetFlipYShader() const {return static_cast<prosper::ShaderFlipY*>(m_hShaderFlipY.get());}

void prosper::GLContext::DoKeepResourceAliveUntilPresentationComplete(const std::shared_ptr<void> &resource) {}
void prosper::GLContext::DoWaitIdle() {glFinish();}
void prosper::GLContext::DoFlushSetupCommandBuffer()
{
	if(m_setupCmdBuffer == nullptr)
		return;
	glFinish();
	m_setupCmdBuffer = nullptr;
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

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glPixelStorei(GL_PACK_ALIGNMENT,1);

	if(IsValidationEnabled())
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback([](GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam) -> void {
			if(severity == GL_DEBUG_SEVERITY_NOTIFICATION || severity == GL_DEBUG_SEVERITY_LOW)
				return;
			// See https://www.khronos.org/opengl/wiki/Debug_Output
			auto *context = static_cast<const GLContext*>(userParam);
			std::string strSource;
			switch(source)
			{
			case GL_DEBUG_SOURCE_API:
				strSource = "OpenGL API";
				break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				strSource = "Window-system API";
				break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				strSource = "Shader compiler";
				break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				strSource = "Third-party";
				break;
			case GL_DEBUG_SOURCE_APPLICATION:
				strSource = "Source application";
				break;
			case GL_DEBUG_SOURCE_OTHER:
			default:
				strSource = "Unknown";
				break;
			}
			auto prosperSeverity = prosper::DebugMessageSeverityFlags::None;
			switch(severity)
			{
			case GL_DEBUG_SEVERITY_NOTIFICATION:
			case GL_DEBUG_SEVERITY_LOW:
				prosperSeverity |= prosper::DebugMessageSeverityFlags::InfoBit;
				break;
			case GL_DEBUG_SEVERITY_MEDIUM:
				prosperSeverity |= prosper::DebugMessageSeverityFlags::WarningBit;
				break;
			case GL_DEBUG_SEVERITY_HIGH:
				prosperSeverity |= prosper::DebugMessageSeverityFlags::ErrorBit;
				break;
			}
			std::string strMsg {message,static_cast<uint64_t>(length)};
			strMsg = "Source: " +strSource +'\n' +strMsg;
			const_cast<GLContext*>(context)->ValidationCallback(prosperSeverity,strMsg);
			},this);
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
	auto img = std::shared_ptr<GLImage>{new GLImage{*this,imgCreateInfo,0,GL_RGBA}};
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
			flags |= GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;
		if(umath::is_flag_set(createInfo.flags,util::BufferCreateInfo::Flags::Persistent))
			flags |= GL_MAP_PERSISTENT_BIT;
		if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::HostCached) && umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::DeviceLocal) == false)
			flags |= GL_CLIENT_STORAGE_BIT;
		if(umath::is_flag_set(createInfo.memoryFeatures,MemoryFeatureFlags::HostAccessable))
			flags |= GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT;
		flags |= GL_DYNAMIC_STORAGE_BIT; // TODO: Only set this flag if we really need it (Let user specify?)
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
	return GLFence::Create(*this);
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
std::shared_ptr<prosper::IImage> prosper::GLContext::CreateImage(const prosper::util::ImageCreateInfo &createInfo,const ImageData &imgData)
{
	return GLImage::Create(*this,createInfo,imgData);
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
std::shared_ptr<prosper::IRenderBuffer> prosper::GLContext::CreateRenderBuffer(
	const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo,const std::vector<prosper::IBuffer*> &buffers,
	const std::vector<prosper::DeviceSize> &offsets,const std::optional<IndexBufferInfo> &indexBufferInfo
)
{
	return std::static_pointer_cast<prosper::IRenderBuffer>(GLRenderBuffer::Create(*this,pipelineCreateInfo,buffers,offsets,indexBufferInfo));
}
bool prosper::GLContext::BindVertexBuffers(const prosper::GraphicsPipelineCreateInfo &pipelineCreateInfo,const std::vector<IBuffer*> &buffers,uint32_t startBinding,const std::vector<DeviceSize> &offsets,uint32_t *optOutAbsAttrId)
{
	std::vector<GLuint> glBuffers;
	std::vector<int64_t> glOffsets;
	std::vector<GLint> glStrides;

	auto numBuffers = buffers.size();
	glBuffers.reserve(numBuffers);
	glOffsets.reserve(numBuffers);
	glStrides.reserve(numBuffers);
	uint32_t absAttrId = 0;
	for(auto i=decltype(startBinding){0u};i<startBinding;++i)
	{
		uint32_t numAttributes;
		pipelineCreateInfo.GetVertexBindingProperties(i,nullptr,nullptr,nullptr,&numAttributes);
		absAttrId += numAttributes;
	}
	for(auto i=decltype(buffers.size()){0u};i<buffers.size();++i)
	{
		auto &buf = buffers.at(i);
		glBuffers.push_back(dynamic_cast<GLBuffer*>(buf)->GetGLBuffer());
		glOffsets.push_back((i < offsets.size()) ? offsets.at(i) : 0);

		uint32_t stride = 0;
		uint32_t bindingIndex = 0;
		uint32_t numAttributes;
		const prosper::VertexInputAttribute *attrs;
		prosper::VertexInputRate rate;
		pipelineCreateInfo.GetVertexBindingProperties(startBinding +i,&bindingIndex,&stride,&rate,&numAttributes,&attrs);
		glStrides.push_back(stride);
		for(auto attrId=decltype(numAttributes){0u};attrId<numAttributes;++attrId)
		{
			auto &attr = attrs[attrId];

			GLboolean normalized = GL_FALSE;
			auto type = util::to_opengl_image_format_type(attr.format,normalized);
			auto numComponents = util::get_component_count(attr.format);

			glEnableVertexAttribArray(absAttrId);
			glBindBuffer(GL_ARRAY_BUFFER,dynamic_cast<GLBuffer*>(buf)->GetGLBuffer());
			int32_t offset = buf->GetStartOffset() +attr.offsetInBytes;
			switch(type)
			{
			case GL_UNSIGNED_BYTE:
			case GL_BYTE:
			case GL_UNSIGNED_SHORT:
			case GL_SHORT:
			case GL_INT:
			case GL_UNSIGNED_INT:
				if(normalized == GL_FALSE)
				{
					glVertexAttribIPointer(
						absAttrId, // Attribute index
						numComponents, // Size
						type, // Type
						stride, // Stride
						(void*)offset // Offset
					);
					break;
				}
				// No break on purpose!
			case GL_FLOAT:
			case GL_HALF_FLOAT:
				glVertexAttribPointer(
					absAttrId, // Attribute index
					numComponents, // Size
					type, // Type
					normalized, // Normalized
					stride, // Stride
					(void*)offset // Offset
				);
				break;
			case GL_DOUBLE:
				glVertexAttribLPointer(
					absAttrId, // Attribute index
					numComponents, // Size
					type, // Type
					stride, // Stride
					(void*)offset // Offset
				);
				break;
			}
			switch(rate)
			{
			case prosper::VertexInputRate::Vertex:
				glVertexAttribDivisor(absAttrId,0);
				break;
			case prosper::VertexInputRate::Instance:
				glVertexAttribDivisor(absAttrId,1);
				break;
			}
			++absAttrId;
		}
	}
	if(optOutAbsAttrId)
		*optOutAbsAttrId = absAttrId;
	return CheckResult();
}
#pragma optimize("",on)
