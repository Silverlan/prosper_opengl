/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_command_buffer.hpp"
#include "gl_context.hpp"
#include "image/gl_image.hpp"
#include "buffers/gl_buffer.hpp"
#include "shader/prosper_shader.hpp"
#include "gl_framebuffer.hpp"
#include "gl_descriptor_set_group.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

using namespace prosper;

std::shared_ptr<ICommandBuffer> GLCommandBuffer::Create(IPrContext &context,prosper::QueueFamilyType queueFamilyType)
{
	return std::shared_ptr<GLCommandBuffer>{new GLCommandBuffer{context,queueFamilyType}};
}

GLCommandBuffer::~GLCommandBuffer()
{

}

bool GLCommandBuffer::Reset(bool shouldReleaseResources) const
{
	return false; // TODO
}
bool GLCommandBuffer::StopRecording() const
{
	return false; // TODO
}

bool GLCommandBuffer::RecordBindIndexBuffer(IBuffer &buf,IndexType indexType,DeviceSize offset)
{
	// glBindVertexBuffer(bindingIndex,buffer,offset,stride);

	return false; // TODO
}

bool GLCommandBuffer::RecordBindVertexBuffers(const prosper::ShaderGraphics &shader,const std::vector<IBuffer*> &buffers,uint32_t startBinding,const std::vector<DeviceSize> &offsets)
{
	std::vector<GLuint> glBuffers;
	std::vector<int64_t> glOffsets;
	std::vector<GLint> glStrides;

	uint32_t pipelineIdx = 0;
	shader.GetBoundPipeline(*this,pipelineIdx);
	auto &createInfo = static_cast<const prosper::GraphicsPipelineCreateInfo&>(*shader.GetPipelineCreateInfo(pipelineIdx));

	auto numBuffers = buffers.size();
	glBuffers.resize(numBuffers);
	glOffsets.resize(numBuffers);
	glStrides.resize(numBuffers);
	for(auto i=decltype(buffers.size()){0u};i<buffers.size();++i)
	{
		auto &buf = buffers.at(i);
		glBuffers.push_back(static_cast<GLBuffer*>(buf)->GetGLBuffer());
		glOffsets.push_back(offsets.at(i));

		uint32_t stride = 0;
		createInfo.GetVertexBindingProperties(startBinding +i,nullptr,&stride);
		glStrides.push_back(stride);
	}

	glBindVertexBuffers(startBinding,numBuffers,glBuffers.data(),glOffsets.data(),glStrides.data());
	return GetContext().CheckResult();
}
bool GLCommandBuffer::RecordDispatchIndirect(prosper::IBuffer &buffer,DeviceSize size)
{
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER,static_cast<GLBuffer&>(buffer).GetGLBuffer());
	glDispatchComputeIndirect(size);
	return GetContext().CheckResult();
}
bool GLCommandBuffer::RecordDraw(uint32_t vertCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordDrawIndexed(uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,int32_t vertexOffset,uint32_t firstInstance)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordDrawIndexedIndirect(IBuffer &buf,DeviceSize offset,uint32_t drawCount,uint32_t stride)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordDrawIndirect(IBuffer &buf,DeviceSize offset,uint32_t count,uint32_t stride)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordFillBuffer(IBuffer &buf,DeviceSize offset,DeviceSize size,uint32_t data)
{

	return false; // TODO
}

bool GLCommandBuffer::RecordSetBlendConstants(const std::array<float,4> &blendConstants)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordSetDepthBounds(float minDepthBounds,float maxDepthBounds)
{

	return false; // TODO
}

bool GLCommandBuffer::RecordSetStencilCompareMask(StencilFaceFlags faceMask,uint32_t stencilCompareMask)
{
	return false; // TODO
}
bool GLCommandBuffer::RecordSetStencilReference(StencilFaceFlags faceMask,uint32_t stencilReference)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordSetStencilWriteMask(StencilFaceFlags faceMask,uint32_t stencilWriteMask)
{
	glStencilMaskSeparate(GL_STENCIL_WRITEMASK,umath::is_flag_set(faceMask,StencilFaceFlags::FrontBit) ? stencilWriteMask : 1);
	glStencilMaskSeparate(GL_STENCIL_BACK_WRITEMASK,umath::is_flag_set(faceMask,StencilFaceFlags::BackBit) ? stencilWriteMask : 1);
	return GetContext().CheckResult();
}

bool GLCommandBuffer::RecordSetDepthBias(float depthBiasConstantFactor,float depthBiasClamp,float depthBiasSlopeFactor)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordClearImage(IImage &img,ImageLayout layout,const std::array<float,4> &clearColor,const prosper::util::ClearImageInfo &clearImageInfo)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordClearImage(IImage &img,ImageLayout layout,float clearDepth,const prosper::util::ClearImageInfo &clearImageInfo)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordClearAttachment(IImage &img,const std::array<float,4> &clearColor,uint32_t attId,uint32_t layerId,uint32_t layerCount)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordClearAttachment(IImage &img,float clearDepth,uint32_t layerId)
{

	return false; // TODO
}
bool GLCommandBuffer::RecordUpdateBuffer(IBuffer &buffer,uint64_t offset,uint64_t size,const void *data)
{
	glNamedBufferSubData(static_cast<GLBuffer&>(buffer).GetGLBuffer(),offset,size,data);
	return GetContext().CheckResult();
}

bool GLCommandBuffer::RecordBindDescriptorSets(PipelineBindPoint bindPoint,prosper::Shader &shader,PipelineID pipelineId,uint32_t firstSet,const std::vector<prosper::IDescriptorSet*> &descSets,const std::vector<uint32_t> dynamicOffsets)
{
	if(true)
		return false;
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
				glBindTextureUnit(0,tex ? static_cast<GLImage&>(tex->GetImage()).GetGLImage() : 0);
				break;
			}
			case prosper::DescriptorSetBinding::Type::UniformBuffer:
			case prosper::DescriptorSetBinding::Type::DynamicUniformBuffer:
			{
				DeviceSize offset,size;
				auto *buf = ds->GetBoundBuffer(j,&offset,&size);
				//static_assert(false,"TODO: Bind where?");
				if(buf)
					;//glBindBufferRange(GL_UNIFORM_BUFFER,blockIndex,static_cast<GLBuffer&>(*buf).GetGLBuffer(),offset,size);
				else
					glBindBufferBase(GL_UNIFORM_BUFFER,0,GL_NONE);
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
bool GLCommandBuffer::RecordPushConstants(prosper::Shader &shader,PipelineID pipelineId,ShaderStageFlags stageFlags,uint32_t offset,uint32_t size,const void *data)
{
	if(true)
		return false;
	auto &buf = GetContext().GetPushConstantBuffer();
	if(RecordUpdateBuffer(buf,offset,size,data) == false)
		return false;
	//glBindBufferRange(GL_UNIFORM_BUFFER,blockIndex,static_cast<GLBuffer&>(buf).GetGLBuffer(),offset,size);
	return GetContext().CheckResult();
}
bool GLCommandBuffer::RecordBindPipeline(PipelineBindPoint in_pipeline_bind_point,PipelineID in_pipeline_id)
{
	if(true)
		return false;
	glUseProgram(in_pipeline_id);
	auto &buf = GetContext().GetPushConstantBuffer();
	//static_assert(false,"Bind push constant buffer");
	return GetContext().CheckResult();
}

bool GLCommandBuffer::RecordSetLineWidth(float lineWidth)
{
	glLineWidth(lineWidth);
	return GetContext().CheckResult();
}
bool GLCommandBuffer::RecordSetViewport(uint32_t width,uint32_t height,uint32_t x,uint32_t y,float minDepth,float maxDepth)
{
	glViewport(x,y,width,height);
	glDepthRangef(minDepth,maxDepth);
	return GetContext().CheckResult();
}
bool GLCommandBuffer::RecordSetScissor(uint32_t width,uint32_t height,uint32_t x,uint32_t y)
{
	glScissor(x,y,width,height);
	return GetContext().CheckResult();
}

GLContext &GLCommandBuffer::GetContext() const {return static_cast<GLContext&>(ICommandBuffer::GetContext());}

GLCommandBuffer::GLCommandBuffer(IPrContext &context,prosper::QueueFamilyType queueFamilyType)
	: ICommandBuffer{context,queueFamilyType}
{}
bool GLCommandBuffer::DoRecordCopyBuffer(const prosper::util::BufferCopy &copyInfo,IBuffer &bufferSrc,IBuffer &bufferDst)
{
	glCopyNamedBufferSubData(static_cast<GLBuffer&>(bufferSrc).GetGLBuffer(),static_cast<GLBuffer&>(bufferDst).GetGLBuffer(),copyInfo.srcOffset,copyInfo.dstOffset,copyInfo.size);
	return GetContext().CheckResult();
}
bool GLCommandBuffer::DoRecordCopyImage(const prosper::util::CopyInfo &copyInfo,IImage &imgSrc,IImage &imgDst,uint32_t w,uint32_t h)
{

	/*glCopyImageSubData(
		static_cast<GLImage&>(imgSrc).GetGLImage(),static_cast<GLImage&>(imgDst).GetGLImage(),copyInfo.srcSubresource.mipLevel,
		x,y,z,,
		);*/
	// TODO

	return false; // TODO
}
bool GLCommandBuffer::DoRecordCopyBufferToImage(const prosper::util::BufferImageCopyInfo &copyInfo,IBuffer &bufferSrc,IImage &imgDst,uint32_t w,uint32_t h)
{

	return false; // TODO
}
bool GLCommandBuffer::DoRecordCopyImageToBuffer(const prosper::util::BufferImageCopyInfo &copyInfo,IImage &imgSrc,ImageLayout srcImageLayout,IBuffer &bufferDst,uint32_t w,uint32_t h)
{

	return false; // TODO
}
bool GLCommandBuffer::DoRecordBlitImage(const prosper::util::BlitInfo &blitInfo,IImage &imgSrc,IImage &imgDst,const std::array<Offset3D,2> &srcOffsets,const std::array<Offset3D,2> &dstOffsets)
{

	return false; // TODO
}
bool GLCommandBuffer::DoRecordResolveImage(IImage &imgSrc,IImage &imgDst,const prosper::util::ImageResolve &resolve)
{

	return false; // TODO
}

//////////////

bool GLPrimaryCommandBuffer::DoRecordBeginRenderPass(
	prosper::IImage &img,prosper::IRenderPass &rp,prosper::IFramebuffer &fb,uint32_t *layerId,
	const std::vector<prosper::ClearValue> &clearValues
)
{
	glBindFramebuffer(GL_FRAMEBUFFER,static_cast<GLFramebuffer&>(fb).GetGLFramebuffer());
	return dynamic_cast<GLContext&>(IPrimaryCommandBuffer::GetContext()).CheckResult();
}
bool GLPrimaryCommandBuffer::StartRecording(bool oneTimeSubmit,bool simultaneousUseAllowed) const
{
	return false; // TODO
}
bool GLPrimaryCommandBuffer::RecordEndRenderPass()
{
	return false; // TODO
}
bool GLPrimaryCommandBuffer::RecordNextSubPass()
{
	return false; // TODO
}
