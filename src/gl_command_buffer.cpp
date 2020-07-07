/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_command_buffer.hpp"
#include "gl_context.hpp"
#include "image/gl_image.hpp"
#include "buffers/gl_buffer.hpp"
#include "shader/prosper_shader.hpp"
#include "shader/gl_shader_clear.hpp"
#include "gl_framebuffer.hpp"
#include "gl_descriptor_set_group.hpp"
#include <sharedutils/scope_guard.h>
#include <shader/prosper_pipeline_create_info.hpp>
#include "gl_api.hpp"
#include "gl_util.hpp"
#include <assert.h>
#pragma optimize("",off)
prosper::GLCommandBuffer::~GLCommandBuffer()
{

}

bool prosper::GLCommandBuffer::Reset(bool shouldReleaseResources) const
{
	return false; // TODO
}
bool prosper::GLCommandBuffer::StopRecording() const
{
	return false; // TODO
}

bool prosper::GLCommandBuffer::RecordBindIndexBuffer(IBuffer &buf,IndexType indexType,DeviceSize offset)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,dynamic_cast<GLBuffer&>(buf).GetGLBuffer());
	return GetContext().CheckResult();
}

bool prosper::GLCommandBuffer::RecordBindVertexBuffers(const prosper::ShaderGraphics &shader,const std::vector<IBuffer*> &buffers,uint32_t startBinding,const std::vector<DeviceSize> &offsets)
{
	std::vector<GLuint> glBuffers;
	std::vector<int64_t> glOffsets;
	std::vector<GLint> glStrides;

	uint32_t pipelineIdx = 0;
	shader.GetBoundPipeline(*this,pipelineIdx);
	auto &createInfo = static_cast<const prosper::GraphicsPipelineCreateInfo&>(*shader.GetPipelineCreateInfo(pipelineIdx));

	auto numBuffers = buffers.size();
	glBuffers.reserve(numBuffers);
	glOffsets.reserve(numBuffers);
	glStrides.reserve(numBuffers);
	uint32_t absAttrId = 0;
	for(auto i=decltype(buffers.size()){0u};i<buffers.size();++i)
	{
		auto &buf = buffers.at(i);
		glBuffers.push_back(dynamic_cast<GLBuffer*>(buf)->GetGLBuffer());
		glOffsets.push_back((i < offsets.size()) ? offsets.at(i) : 0);

		uint32_t stride = 0;
		uint32_t bindingIndex = 0;
		uint32_t numAttributes;
		const prosper::VertexInputAttribute *attrs;
		createInfo.GetVertexBindingProperties(startBinding +i,&bindingIndex,&stride,nullptr,&numAttributes,&attrs);
		glStrides.push_back(stride);
		for(auto attrId=decltype(numAttributes){0u};attrId<numAttributes;++attrId)
		{
			auto &attr = attrs[attrId];
			// TODO: Use VAOs instead
			glEnableVertexAttribArray(i);
			glBindBuffer(GL_ARRAY_BUFFER,dynamic_cast<GLBuffer*>(buf)->GetGLBuffer());
			int32_t offset = attr.offsetInBytes;
			glVertexAttribPointer(
				absAttrId++, // Attribute index
				prosper::util::get_byte_size(attr.format) /sizeof(float), // Size
				GL_FLOAT, // Type
				GL_FALSE, // Normalized
				stride, // Stride
				(void*)offset // Offset
			);
		}
	}

	//glBindVertexBuffers(startBinding,numBuffers,glBuffers.data(),glOffsets.data(),glStrides.data());
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordDispatchIndirect(prosper::IBuffer &buffer,DeviceSize size)
{
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER,dynamic_cast<GLBuffer&>(buffer).GetGLBuffer());
	glDispatchComputeIndirect(size);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordDraw(uint32_t vertCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance)
{
	if(m_boundPipelineData.shader.expired() || m_boundPipelineData.pipelineId.has_value() == false || m_boundPipelineData.shader->IsGraphicsShader() == false)
		return false;
	auto &pipelineCreateInfo = *static_cast<prosper::GraphicsPipelineCreateInfo*>(static_cast<ShaderGraphics*>(m_boundPipelineData.shader.get())->GetPipelineCreateInfo(*m_boundPipelineData.shaderPipelineId));
	auto glTopology = prosper::util::to_opengl_enum(pipelineCreateInfo.GetPrimitiveTopology());
	glDrawArraysInstancedBaseInstance(glTopology,firstVertex,vertCount,instanceCount,firstInstance);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordDrawIndexed(uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,uint32_t firstInstance)
{
#if 0
	prosper::ShaderGraphics *x;
	GLenum topology;
	prosper::VertexInputAttribute vertexInputAttribute;
	static_cast<prosper::GraphicsPipelineCreateInfo*>(x->GetPipelineCreateInfo(0))->GetVertexBindingProperties(
		0,nullptr,nullptr,nullptr,nullptr,&vertexInputAttribute,nullptr
	);

	glDrawElements(topology,indexCount,GL_UNSIGNED_SHORT,nullptr);
	return GetContext().CheckResult();
#endif
	return false;
}
bool prosper::GLCommandBuffer::RecordDrawIndexedIndirect(IBuffer &buf,DeviceSize offset,uint32_t drawCount,uint32_t stride)
{

	return false; // TODO
}
bool prosper::GLCommandBuffer::RecordDrawIndirect(IBuffer &buf,DeviceSize offset,uint32_t count,uint32_t stride)
{

	return false; // TODO
}
bool prosper::GLCommandBuffer::RecordFillBuffer(IBuffer &buf,DeviceSize offset,DeviceSize size,uint32_t value)
{
	// TODO: Allow VK_WHOLE_SIZE as size?
	assert((size %sizeof(uint32_t) == 0));
	std::vector<uint32_t> vData {};
	vData.resize(size /sizeof(uint32_t));
	std::fill(vData.begin(),vData.end(),value);
	glClearNamedBufferSubData(dynamic_cast<GLBuffer&>(buf).GetGLBuffer(),GL_R32UI,offset,size,GL_RGBA_INTEGER,GL_UNSIGNED_INT,vData.data());
	return GetContext().CheckResult();
}

bool prosper::GLCommandBuffer::RecordSetBlendConstants(const std::array<float,4> &blendConstants)
{

	return false; // TODO
}
bool prosper::GLCommandBuffer::RecordSetDepthBounds(float minDepthBounds,float maxDepthBounds)
{

	return false; // TODO
}

bool prosper::GLCommandBuffer::RecordSetStencilCompareMask(StencilFaceFlags faceMask,uint32_t stencilCompareMask)
{
	return false; // TODO
}
bool prosper::GLCommandBuffer::RecordSetStencilReference(StencilFaceFlags faceMask,uint32_t stencilReference)
{

	return false; // TODO
}
bool prosper::GLCommandBuffer::RecordSetStencilWriteMask(StencilFaceFlags faceMask,uint32_t stencilWriteMask)
{
	glStencilMaskSeparate(GL_STENCIL_WRITEMASK,umath::is_flag_set(faceMask,StencilFaceFlags::FrontBit) ? stencilWriteMask : 1);
	glStencilMaskSeparate(GL_STENCIL_BACK_WRITEMASK,umath::is_flag_set(faceMask,StencilFaceFlags::BackBit) ? stencilWriteMask : 1);
	return GetContext().CheckResult();
}

bool prosper::GLCommandBuffer::RecordPipelineBarrier(const prosper::util::PipelineBarrierInfo &barrierInfo)
{
	return true; // TODO
}

bool prosper::GLCommandBuffer::RecordSetDepthBias(float depthBiasConstantFactor,float depthBiasClamp,float depthBiasSlopeFactor)
{

	return false; // TODO
}
static void clear_image(prosper::GLContext &context,prosper::IImage &img,uint32_t layerId,uint32_t layerCount,uint32_t baseMipmap,uint32_t mipmapCount,const std::array<float,4> &clearColor,float clearDepth,bool depth)
{
	GLint drawFboId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&drawFboId);
	ScopeGuard sg {[drawFboId]() {
		// Restore previous bound framebuffer
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,drawFboId);
	}};

	auto framebuffer = static_cast<prosper::GLImage&>(img).GetOrCreateFramebuffer(layerId,layerCount,baseMipmap,mipmapCount);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,static_cast<prosper::GLFramebuffer&>(*framebuffer).GetGLFramebuffer());
	if(depth == false)
	{
		glClearColor(clearColor.at(0),clearColor.at(1),clearColor.at(2),clearColor.at(3));
		context.CheckResult();
		glClear(GL_COLOR_BUFFER_BIT);
		context.CheckResult();
	}
	else
	{
		glClearDepth(depth);
		glClear(GL_DEPTH_BUFFER_BIT);
	}
}
bool prosper::GLCommandBuffer::RecordClearImage(IImage &img,ImageLayout layout,const std::array<float,4> &clearColor,const prosper::util::ClearImageInfo &clearImageInfo)
{
	if(IsPrimary() == false)
		return false;
	clear_image(GetContext(),img,clearImageInfo.subresourceRange.baseArrayLayer,clearImageInfo.subresourceRange.layerCount,clearImageInfo.subresourceRange.baseMipLevel,clearImageInfo.subresourceRange.levelCount,clearColor,0.f,false);
	return GetContext().CheckResult();
#if 0
	auto vClearColor = Vector4{clearColor.at(0),clearColor.at(1),clearColor.at(2),clearColor.at(3)};
	std::vector<Vector4> data {};
	for(auto i=clearImageInfo.subresourceRange.baseMipLevel;i<(clearImageInfo.subresourceRange.baseMipLevel +clearImageInfo.subresourceRange.levelCount);++i)
	{
		// TODO: This is *very* inefficient, how can we handle this better?
		auto w = img.GetWidth(i);
		auto h = img.GetHeight(i);
		data.resize(w *h);
		std::fill(data.begin(),data.end(),vClearColor);
		glClearTexSubImage(
			static_cast<GLImage&>(img).GetGLImage(),clearImageInfo.subresourceRange.baseMipLevel,0,0,clearImageInfo.subresourceRange.baseArrayLayer,
			w,h,clearImageInfo.subresourceRange.layerCount,GL_RGBA,GL_FLOAT,data.data()
		);
	}
	return GetContext().CheckResult();
#endif
}
bool prosper::GLCommandBuffer::RecordClearImage(IImage &img,ImageLayout layout,float clearDepth,const prosper::util::ClearImageInfo &clearImageInfo)
{
	if(IsPrimary() == false)
		return false;
	clear_image(GetContext(),img,clearImageInfo.subresourceRange.baseArrayLayer,clearImageInfo.subresourceRange.layerCount,clearImageInfo.subresourceRange.baseMipLevel,clearImageInfo.subresourceRange.levelCount,{},clearDepth,true);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordClearAttachment(IImage &img,const std::array<float,4> &clearColor,uint32_t attId,uint32_t layerId,uint32_t layerCount)
{
	if(IsPrimary() == false)
		return false;
	clear_image(GetContext(),img,layerId,layerCount,0,std::numeric_limits<uint32_t>::max(),clearColor,0.f,false);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordClearAttachment(IImage &img,float clearDepth,uint32_t layerId)
{
	if(IsPrimary() == false)
		return false;
	clear_image(GetContext(),img,layerId,1,0,std::numeric_limits<uint32_t>::max(),{},clearDepth,true);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordUpdateBuffer(IBuffer &buffer,uint64_t offset,uint64_t size,const void *data)
{
	glNamedBufferSubData(dynamic_cast<GLBuffer&>(buffer).GetGLBuffer(),offset,size,data);
	return GetContext().CheckResult();
}

bool prosper::GLCommandBuffer::RecordBindDescriptorSets(PipelineBindPoint bindPoint,prosper::Shader &shader,PipelineID pipelineId,uint32_t firstSet,const std::vector<prosper::IDescriptorSet*> &descSets,const std::vector<uint32_t> dynamicOffsets)
{
	auto &createInfo = *shader.GetPipelineCreateInfo(pipelineId);
	auto &dsInfoItems = *createInfo.GetDsCreateInfoItems();
	auto numSets = descSets.size();
	// TODO
	//GLuint blockIndex = glGetUniformBlockIndex(programHandle,
	//	"BlobSettings");
	for(auto i=decltype(numSets){0u};i<numSets;++i)
	{
		auto setIdx = firstSet +i;
		auto &dsCreateInfo = dsInfoItems.at(setIdx);
		auto *ds = descSets.at(i);
		auto numBindings = ds->GetBindingCount();
		for(auto j=decltype(numBindings){0u};j<numBindings;++j)
		{
			auto &binding = *ds->GetBinding(j);
			auto bindingIdx = binding.GetBindingIndex();
			auto layoutLocation = prosper::util::get_layout_location(setIdx,j);
			auto type = binding.GetType();
			switch(type)
			{
			case prosper::DescriptorSetBinding::Type::Texture:
			case prosper::DescriptorSetBinding::Type::ArrayTexture:
			{
				std::optional<uint32_t> layer {};
				auto *tex = ds->GetBoundTexture(j,&layer);
				if(layer.has_value() && GetContext().IsValidationEnabled())
					GetContext().ValidationCallback(DebugMessageSeverityFlags::WarningBit,"Attempted to bind layer " +std::to_string(*layer) +" of texture '" +tex->GetDebugName() +"'! This is not allowed in OpenGL!");
				//static_assert(false,"TODO: Bind where?");
				auto it = m_boundPipelineData.shaderActiveTexturePerLocation.find(layoutLocation);
				uint32_t activeTextureSlot;
				if(it != m_boundPipelineData.shaderActiveTexturePerLocation.end())
					activeTextureSlot = it->second;
				else
				{
					activeTextureSlot = m_boundPipelineData.nextActiveTextureIndex++;
					m_boundPipelineData.shaderActiveTexturePerLocation[layoutLocation] = activeTextureSlot;
				}
				glBindTextureUnit(activeTextureSlot,tex ? static_cast<GLImage&>(tex->GetImage()).GetGLImage() : 0);
				//glUniform1i(layoutLocation,activeTextureSlot);
				break;
			}
			case prosper::DescriptorSetBinding::Type::UniformBuffer:
			case prosper::DescriptorSetBinding::Type::DynamicUniformBuffer:
			{
				DeviceSize offset,size;
				auto *buf = ds->GetBoundBuffer(j,&offset,&size);
				//static_assert(false,"TODO: Bind where?");
				if(buf)
					glBindBufferBase(GL_UNIFORM_BUFFER,layoutLocation,dynamic_cast<GLBuffer&>(*buf).GetGLBuffer());
					//;//glBindBufferRange(GL_UNIFORM_BUFFER,blockIndex,static_cast<GLBuffer&>(*buf).GetGLBuffer(),offset,size);
				else
					;//glBindBufferBase(GL_UNIFORM_BUFFER,layoutLocation,dynamic_cast<GLBuffer&>(*buf).GetGLBuffer());
				//glUniform1i(2,2); // Location -> Binding
				break;
			}
			case prosper::DescriptorSetBinding::Type::StorageBuffer:
			{
				DeviceSize offset,size;
				auto *buf = ds->GetBoundBuffer(j,&offset,&size);
				//static_assert(false,"TODO: Bind where?");
				if(buf)
					;//glBindBufferRange(GL_SHADER_STORAGE_BUFFER,blockIndex,static_cast<GLBuffer&>(*buf).GetGLBuffer(),offset,size);
				else
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,GL_NONE);
				break;
			}
			}
		}
	}
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordPushConstants(prosper::Shader &shader,PipelineID pipelineId,ShaderStageFlags stageFlags,uint32_t offset,uint32_t size,const void *data)
{
	auto &buf = GetContext().GetPushConstantBuffer();
	if(RecordUpdateBuffer(buf,offset,size,data) == false)
		return false;
	glBindBufferBase(GL_UNIFORM_BUFFER,0,buf.GetGLBuffer());
	return GetContext().CheckResult();
}
void prosper::GLCommandBuffer::ClearBoundPipeline()
{
	ICommandBuffer::ClearBoundPipeline();
	m_boundPipelineData.pipelineId = {};
	m_boundPipelineData.shader = {};
	m_boundPipelineData.shaderPipelineId = {};
	m_boundPipelineData.nextActiveTextureIndex = 0;
	m_boundPipelineData.shaderActiveTexturePerLocation.clear();
}
std::optional<prosper::PipelineID> prosper::GLCommandBuffer::GetBoundPipelineId() const {return m_boundPipelineData.pipelineId;}
prosper::Shader *prosper::GLCommandBuffer::GetBoundShader() const {return m_boundPipelineData.shader.get();}
bool prosper::GLCommandBuffer::DoRecordBindShaderPipeline(prosper::Shader &shader,PipelineID shaderPipelineId,PipelineID pipelineId)
{
	auto program = GetContext().GetPipelineProgram(pipelineId);
	if(program.has_value() == false)
		return false;
	glUseProgram(*program);

	if(shader.IsGraphicsShader())
	{
		auto *createInfo = static_cast<prosper::GraphicsPipelineCreateInfo*>(shader.GetPipelineCreateInfo(shaderPipelineId));
		if(createInfo == nullptr)
			return false;
		bool blendingEnabled;
		prosper::BlendOp blendOpColor,blendOpAlpha;
		prosper::BlendFactor srcColorBlendFactor,dstColorBlendFactor,srcAlphaBlendFactor,dstAlphaBlendFactor;
		prosper::ColorComponentFlags channelWriteMask;
		if(createInfo->GetColorBlendAttachmentProperties(
			0 /* sub-pass id */,&blendingEnabled,
			&blendOpColor,&blendOpAlpha,
			&srcColorBlendFactor,&dstColorBlendFactor,
			&srcAlphaBlendFactor,&dstAlphaBlendFactor,
			&channelWriteMask
		) == false)
			return false;
		
		if(blendingEnabled)
		{
			glEnable(GL_BLEND);
			glBlendEquationSeparate(util::to_opengl_enum(blendOpColor),util::to_opengl_enum(blendOpAlpha));
			glBlendFuncSeparate(
				util::to_opengl_enum(srcColorBlendFactor),util::to_opengl_enum(dstColorBlendFactor),
				util::to_opengl_enum(srcAlphaBlendFactor),util::to_opengl_enum(dstAlphaBlendFactor)
			);
			glColorMask(
				umath::is_flag_set(channelWriteMask,prosper::ColorComponentFlags::RBit),
				umath::is_flag_set(channelWriteMask,prosper::ColorComponentFlags::GBit),
				umath::is_flag_set(channelWriteMask,prosper::ColorComponentFlags::BBit),
				umath::is_flag_set(channelWriteMask,prosper::ColorComponentFlags::ABit)
			);
		}
		else
			glDisable(GL_BLEND);
	}

	auto &buf = GetContext().GetPushConstantBuffer();
	// TODO
	//static_assert(false,"Bind push constant buffer");
	auto result = GetContext().CheckResult();
	if(result)
	{
		m_boundPipelineData.pipelineId = pipelineId;
		m_boundPipelineData.shader = shader.GetHandle();
		m_boundPipelineData.shaderPipelineId = shaderPipelineId;
	}
	return result;
}

bool prosper::GLCommandBuffer::RecordSetLineWidth(float lineWidth)
{
	glLineWidth(lineWidth);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordSetViewport(uint32_t width,uint32_t height,uint32_t x,uint32_t y,float minDepth,float maxDepth)
{
	glViewport(x,y,width,height);
	glDepthRangef(minDepth,maxDepth);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordSetScissor(uint32_t width,uint32_t height,uint32_t x,uint32_t y)
{
	glScissor(x,y,width,height);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordBeginPipelineStatisticsQuery(const PipelineStatisticsQuery &query) const
{

	return false; // TODO
}
bool prosper::GLCommandBuffer::RecordEndPipelineStatisticsQuery(const PipelineStatisticsQuery &query) const
{

	return false; // TODO
}
bool prosper::GLCommandBuffer::RecordBeginOcclusionQuery(const OcclusionQuery &query) const
{

	return false; // TODO
}
bool prosper::GLCommandBuffer::RecordEndOcclusionQuery(const OcclusionQuery &query) const
{

	return false; // TODO
}
bool prosper::GLCommandBuffer::WriteTimestampQuery(const TimestampQuery &query) const
{

	return false; // TODO
}
bool prosper::GLCommandBuffer::ResetQuery(const Query &query) const
{

	return false; // TODO
}

prosper::GLContext &prosper::GLCommandBuffer::GetContext() const {return static_cast<GLContext&>(ICommandBuffer::GetContext());}

prosper::GLCommandBuffer::GLCommandBuffer(IPrContext &context,prosper::QueueFamilyType queueFamilyType)
	: ICommandBuffer{context,queueFamilyType}
{}
bool prosper::GLCommandBuffer::DoRecordCopyBuffer(const prosper::util::BufferCopy &copyInfo,IBuffer &bufferSrc,IBuffer &bufferDst)
{
	glCopyNamedBufferSubData(dynamic_cast<GLBuffer&>(bufferSrc).GetGLBuffer(),dynamic_cast<GLBuffer&>(bufferDst).GetGLBuffer(),copyInfo.srcOffset,copyInfo.dstOffset,copyInfo.size);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::DoRecordCopyImage(const prosper::util::CopyInfo &copyInfo,IImage &imgSrc,IImage &imgDst,uint32_t w,uint32_t h)
{

	/*glCopyImageSubData(
		static_cast<GLImage&>(imgSrc).GetGLImage(),static_cast<GLImage&>(imgDst).GetGLImage(),copyInfo.srcSubresource.mipLevel,
		x,y,z,,
		);*/
	// TODO

	return false; // TODO
}

bool prosper::GLCommandBuffer::DoRecordCopyBufferToImage(const prosper::util::BufferImageCopyInfo &copyInfo,IBuffer &bufferSrc,IImage &imgDst,uint32_t w,uint32_t h)
{
	glBindTexture(GL_TEXTURE_2D,static_cast<GLImage&>(imgDst).GetGLImage());
	if(prosper::util::is_compressed_format(imgDst.GetFormat()))
	{
		GLint size;
		glGetTexLevelParameteriv(GL_TEXTURE_2D,copyInfo.mipLevel,GL_TEXTURE_COMPRESSED_IMAGE_SIZE,&size);

		std::vector<uint8_t> imgData {};
		imgData.resize(size);
		bufferSrc.Read(0,imgData.size(),imgData.data());

		auto format = prosper::util::to_opengl_image_format(imgDst.GetFormat());
		glCompressedTexSubImage2D(
			GL_TEXTURE_2D,copyInfo.mipLevel,0,0,w,h,format,imgData.size(),imgData.data()
		);
		return GetContext().CheckResult();
	}
	auto size = w *h *prosper::util::get_byte_size(imgDst.GetFormat());
	std::vector<uint8_t> imgData {};
	imgData.resize(size);
	bufferSrc.Read(0,imgData.size(),imgData.data());
	glTexSubImage2D(
		GL_TEXTURE_2D,copyInfo.mipLevel,0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,imgData.data()
	);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::DoRecordCopyImageToBuffer(const prosper::util::BufferImageCopyInfo &copyInfo,IImage &imgSrc,ImageLayout srcImageLayout,IBuffer &bufferDst,uint32_t w,uint32_t h)
{

	return false; // TODO
}
bool prosper::GLCommandBuffer::DoRecordBlitImage(const prosper::util::BlitInfo &blitInfo,IImage &imgSrc,IImage &imgDst,const std::array<Offset3D,2> &srcOffsets,const std::array<Offset3D,2> &dstOffsets)
{
	auto framebufferSrc = static_cast<GLImage&>(imgSrc).GetOrCreateFramebuffer(blitInfo.srcSubresourceLayer.baseArrayLayer,blitInfo.srcSubresourceLayer.layerCount,blitInfo.srcSubresourceLayer.mipLevel,1);
	auto framebufferDst = static_cast<GLImage&>(imgDst).GetOrCreateFramebuffer(blitInfo.dstSubresourceLayer.baseArrayLayer,blitInfo.dstSubresourceLayer.layerCount,blitInfo.dstSubresourceLayer.mipLevel,1);
	glBlitNamedFramebuffer(
		framebufferSrc->GetGLFramebuffer(),framebufferDst->GetGLFramebuffer(),
		srcOffsets.at(0).x,srcOffsets.at(0).y,srcOffsets.at(1).x,srcOffsets.at(1).y,
		dstOffsets.at(0).x,dstOffsets.at(0).y,dstOffsets.at(1).x,dstOffsets.at(1).y,
		GL_COLOR_BUFFER_BIT,GL_LINEAR
	);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::DoRecordResolveImage(IImage &imgSrc,IImage &imgDst,const prosper::util::ImageResolve &resolve)
{

	return false; // TODO
}

//////////////

std::shared_ptr<prosper::GLPrimaryCommandBuffer> prosper::GLPrimaryCommandBuffer::Create(IPrContext &context,prosper::QueueFamilyType queueFamilyType)
{
	return std::shared_ptr<GLPrimaryCommandBuffer>{new GLPrimaryCommandBuffer{context,queueFamilyType}};
}
bool prosper::GLPrimaryCommandBuffer::IsPrimary() const {return true;}
prosper::GLPrimaryCommandBuffer::GLPrimaryCommandBuffer(IPrContext &context,prosper::QueueFamilyType queueFamilyType)
	: GLCommandBuffer{context,queueFamilyType},
	ICommandBuffer{context,queueFamilyType}
{}
bool prosper::GLPrimaryCommandBuffer::DoRecordBeginRenderPass(
	prosper::IImage &img,prosper::IRenderPass &rp,prosper::IFramebuffer &fb,uint32_t *layerId,
	const std::vector<prosper::ClearValue> &clearValues
)
{
	glBindFramebuffer(GL_FRAMEBUFFER,static_cast<GLFramebuffer&>(fb).GetGLFramebuffer());
	return dynamic_cast<GLContext&>(IPrimaryCommandBuffer::GetContext()).CheckResult();
}
bool prosper::GLPrimaryCommandBuffer::StartRecording(bool oneTimeSubmit,bool simultaneousUseAllowed) const
{
	return true;
}
bool prosper::GLPrimaryCommandBuffer::RecordEndRenderPass()
{
	return true; // TODO
}
bool prosper::GLPrimaryCommandBuffer::RecordNextSubPass()
{
	return false; // TODO
}

std::shared_ptr<prosper::GLSecondaryCommandBuffer> prosper::GLSecondaryCommandBuffer::Create(IPrContext &context,prosper::QueueFamilyType queueFamilyType)
{
	return std::shared_ptr<GLSecondaryCommandBuffer>{new GLSecondaryCommandBuffer{context,queueFamilyType}};
}
bool prosper::GLSecondaryCommandBuffer::IsSecondary() const {return true;}
prosper::GLSecondaryCommandBuffer::GLSecondaryCommandBuffer(IPrContext &context,prosper::QueueFamilyType queueFamilyType)
	: GLCommandBuffer{context,queueFamilyType},
	ICommandBuffer{context,queueFamilyType},ISecondaryCommandBuffer{context,queueFamilyType}
{}
#pragma optimize("",on)
