// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include <prosper_util.hpp>
#include "gl_command_buffer.hpp"
#include "gl_context.hpp"
#include "image/gl_image.hpp"
#include "image/gl_image_view.hpp"
#include "image/gl_sampler.hpp"
#include "buffers/gl_buffer.hpp"
#include "buffers/gl_render_buffer.hpp"
#include "shader/prosper_shader.hpp"
#include "shader/gl_shader_blit.hpp"
#include "shader/prosper_shader_flip_image.hpp"
#include "gl_render_pass.hpp"
#include "gl_framebuffer.hpp"
#include "gl_descriptor_set_group.hpp"
#include <sharedutils/scope_guard.h>
#include <shader/prosper_pipeline_create_info.hpp>
#include "gl_api.hpp"
#include "gl_util.hpp"
#include <assert.h>
#include <image/prosper_texture.hpp>

static const auto SCISSOR_FLIP_Y = false;

namespace prosper {
	class DLLPROSPER_GL GLShaderPipelineLayout : public IShaderPipelineLayout {
	  public:
		static std::unique_ptr<GLShaderPipelineLayout> Create(const Shader &shader, uint32_t pipelineIdx);
		prosper::Shader *GetShader() const { return m_shader.get(); }
		uint32_t GetPipelineIndex() const { return m_pipelineIdx; }
	  private:
		using IShaderPipelineLayout::IShaderPipelineLayout;
		mutable ::util::WeakHandle<prosper::Shader> m_shader {};
		mutable uint32_t m_pipelineIdx = 0;
	};
};

std::unique_ptr<prosper::GLShaderPipelineLayout> prosper::GLShaderPipelineLayout::Create(const Shader &shader, uint32_t pipelineIdx)
{
	auto res = std::unique_ptr<GLShaderPipelineLayout> {new GLShaderPipelineLayout {}};
	res->m_shader = const_cast<Shader &>(shader).GetHandle();
	res->m_pipelineIdx = pipelineIdx;
	return res;
}
std::unique_ptr<prosper::IShaderPipelineLayout> prosper::GLContext::GetShaderPipelineLayout(const Shader &shader, uint32_t pipelineIdx) const { return GLShaderPipelineLayout::Create(shader, pipelineIdx); }

////////////////

prosper::GLCommandBuffer::~GLCommandBuffer() {}

bool prosper::GLCommandBuffer::Reset(bool shouldReleaseResources) const
{
	return true; // TODO
}
bool prosper::GLCommandBuffer::StopRecording() const { return true; }

bool prosper::GLCommandBuffer::RecordBindIndexBuffer(IBuffer &buf, IndexType indexType, DeviceSize offset)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf.GetAPITypeRef<GLBuffer>().GetGLBuffer());
	m_boundIndexBufferData.indexType = indexType;
	m_boundIndexBufferData.offset = buf.GetStartOffset() + offset;
	return GetContext().CheckResult();
}

bool prosper::GLCommandBuffer::RecordBindVertexBuffer(const prosper::ShaderGraphics &shader, const IBuffer &buf, uint32_t startBinding, DeviceSize offset) { return RecordBindVertexBuffers(shader, {&const_cast<IBuffer &>(buf)}, startBinding, {offset}); }

bool prosper::GLCommandBuffer::RecordBindVertexBuffers(const prosper::ShaderGraphics &shader, const std::vector<IBuffer *> &buffers, uint32_t startBinding, const std::vector<DeviceSize> &offsets)
{
	uint32_t pipelineIdx = 0;
	shader.GetBoundPipeline(*this, pipelineIdx);
	auto &createInfo = static_cast<const prosper::GraphicsPipelineCreateInfo &>(*shader.GetPipelineCreateInfo(pipelineIdx));
	uint32_t absAttrId;
	auto result = GetContext().BindVertexBuffers(createInfo, buffers, startBinding, offsets, &absAttrId);
	m_boundPipelineData.numVertexAttribBindings = umath::max(absAttrId, m_boundPipelineData.numVertexAttribBindings);
	return result;
}
bool prosper::GLCommandBuffer::RecordBindRenderBuffer(const IRenderBuffer &renderBuffer)
{
	glBindVertexArray(static_cast<const GLRenderBuffer &>(renderBuffer).GetGLVertexArrayObject());
	auto *indexBufferInfo = renderBuffer.GetIndexBufferInfo();
	if(indexBufferInfo) {
		m_boundIndexBufferData.indexType = indexBufferInfo->indexType;
		m_boundIndexBufferData.offset = indexBufferInfo->buffer->GetStartOffset() + indexBufferInfo->offset;
	}
	m_boundPipelineData.numVertexAttribBindings = 0;
	return true;
}
bool prosper::GLCommandBuffer::RecordDispatchIndirect(prosper::IBuffer &buffer, DeviceSize size)
{
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer.GetAPITypeRef<GLBuffer>().GetGLBuffer());
	glDispatchComputeIndirect(size);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordDispatch(uint32_t x, uint32_t y, uint32_t z)
{
	glDispatchCompute(x, y, z);
	return GetContext().CheckResult();
}
void prosper::GLCommandBuffer::CheckViewportAndScissorBounds() const
{
	if(GetContext().IsValidationEnabled() == false)
		return;
	std::array<GLint, 4> viewport {};
	glGetIntegerv(GL_VIEWPORT, viewport.data());
	if(viewport != m_viewport)
		GetContext().ValidationCallback(DebugMessageSeverityFlags::WarningBit, "Currently assigned viewport bounds do not match expected viewport bounds!");

	GLboolean scissorTest = GL_FALSE;
	glGetBooleanv(GL_SCISSOR_TEST, &scissorTest);
	if(scissorTest == GL_TRUE) {
		std::array<GLint, 4> scissorBox {};
		glGetIntegerv(GL_SCISSOR_BOX, scissorBox.data());
		if constexpr(SCISSOR_FLIP_Y)
			scissorBox.at(1) = GetContext().GetWindowHeight() - scissorBox.at(1) - scissorBox.at(3); // y is flipped
		if(scissorBox != m_scissor)
			GetContext().ValidationCallback(DebugMessageSeverityFlags::WarningBit, "Currently assigned scissor bounds do not match expected scissor bounds!");
	}
}
bool prosper::GLCommandBuffer::RecordDraw(uint32_t vertCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	if(m_boundPipelineData.shader.expired() || m_boundPipelineData.pipelineId.has_value() == false || m_boundPipelineData.shader->IsGraphicsShader() == false)
		return false;
	CheckViewportAndScissorBounds();
	auto &pipelineCreateInfo = *static_cast<prosper::GraphicsPipelineCreateInfo *>(static_cast<ShaderGraphics *>(m_boundPipelineData.shader.get())->GetPipelineCreateInfo(*m_boundPipelineData.shaderPipelineId));
	auto glTopology = prosper::util::to_opengl_enum(pipelineCreateInfo.GetPrimitiveTopology());
	glDrawArraysInstancedBaseInstance(glTopology, firstVertex, vertCount, instanceCount, firstInstance);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance)
{
	if(m_boundPipelineData.shader.expired() || m_boundPipelineData.pipelineId.has_value() == false || m_boundPipelineData.shader->IsGraphicsShader() == false)
		return false;
	CheckViewportAndScissorBounds();
	GetContext().CheckResult();
	auto &pipelineCreateInfo = *static_cast<prosper::GraphicsPipelineCreateInfo *>(static_cast<ShaderGraphics *>(m_boundPipelineData.shader.get())->GetPipelineCreateInfo(*m_boundPipelineData.shaderPipelineId));
	auto glTopology = prosper::util::to_opengl_enum(pipelineCreateInfo.GetPrimitiveTopology());

	int32_t offset = m_boundIndexBufferData.offset;
	if(instanceCount == 1)
		glDrawElements(glTopology, indexCount, util::to_opengl_enum(m_boundIndexBufferData.indexType), (void *)offset);
	else
		glDrawElementsInstanced(glTopology, indexCount, util::to_opengl_enum(m_boundIndexBufferData.indexType), (void *)offset, instanceCount);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordDrawIndexedIndirect(IBuffer &buf, DeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf.GetAPITypeRef<GLBuffer>().GetGLBuffer());
	for(uint32_t i = 0; i < drawCount; ++i)
		glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<void *>(offset + i * stride));
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	return true;
}
bool prosper::GLCommandBuffer::RecordDrawIndirect(IBuffer &buf, DeviceSize offset, uint32_t count, uint32_t stride)
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf.GetAPITypeRef<GLBuffer>().GetGLBuffer());
	for(uint32_t i = 0; i < count; ++i)
		glDrawArraysIndirect(GL_TRIANGLES, reinterpret_cast<void *>(offset + i * stride));
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	return true;
}
bool prosper::GLCommandBuffer::RecordFillBuffer(IBuffer &buf, DeviceSize offset, DeviceSize size, uint32_t value)
{
	// TODO: Allow VK_WHOLE_SIZE as size?
	assert((size % sizeof(uint32_t) == 0));
	std::vector<uint32_t> vData {};
	vData.resize(size / sizeof(uint32_t));
	std::fill(vData.begin(), vData.end(), value);
	glClearNamedBufferSubData(buf.GetAPITypeRef<GLBuffer>().GetGLBuffer(), GL_R32UI, offset, size, GL_RGBA_INTEGER, GL_UNSIGNED_INT, vData.data());
	return GetContext().CheckResult();
}

bool prosper::GLCommandBuffer::RecordSetBlendConstants(const std::array<float, 4> &blendConstants)
{
	glBlendColor(blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]);
	return true;
}
bool prosper::GLCommandBuffer::RecordSetDepthBounds(float minDepthBounds, float maxDepthBounds)
{
	// Note: This is not equivalent to Vulkan
	glDepthRange(minDepthBounds, maxDepthBounds);
	return true;
}

bool prosper::GLCommandBuffer::RecordSetStencilCompareMask(StencilFaceFlags faceMask, uint32_t stencilCompareMask)
{
	GLint func, ref;

	if(umath::is_flag_set(faceMask, StencilFaceFlags::FrontBit)) {
		glGetIntegerv(GL_STENCIL_FUNC, &func);
		glGetIntegerv(GL_STENCIL_REF, &ref);
		glStencilFuncSeparate(GL_FRONT, func, ref, stencilCompareMask);
	}

	if(umath::is_flag_set(faceMask, StencilFaceFlags::BackBit)) {
		glGetIntegerv(GL_STENCIL_BACK_FUNC, &func);
		glGetIntegerv(GL_STENCIL_BACK_REF, &ref);
		glStencilFuncSeparate(GL_BACK, func, ref, stencilCompareMask);
	}
	return true;
}
bool prosper::GLCommandBuffer::RecordSetStencilReference(StencilFaceFlags faceMask, uint32_t stencilReference)
{
	GLint func, mask;
	if(umath::is_flag_set(faceMask, StencilFaceFlags::FrontBit)) {
		glGetIntegerv(GL_STENCIL_FUNC, &func);
		glGetIntegerv(GL_STENCIL_VALUE_MASK, &mask);
		glStencilFuncSeparate(GL_FRONT, func, stencilReference, mask);
	}

	if(umath::is_flag_set(faceMask, StencilFaceFlags::BackBit)) {
		glGetIntegerv(GL_STENCIL_BACK_FUNC, &func);
		glGetIntegerv(GL_STENCIL_BACK_VALUE_MASK, &mask);
		glStencilFuncSeparate(GL_BACK, func, stencilReference, mask);
	}
	return true;
}
bool prosper::GLCommandBuffer::RecordSetStencilWriteMask(StencilFaceFlags faceMask, uint32_t stencilWriteMask)
{
	glStencilMaskSeparate(GL_STENCIL_WRITEMASK, umath::is_flag_set(faceMask, StencilFaceFlags::FrontBit) ? stencilWriteMask : 1);
	glStencilMaskSeparate(GL_STENCIL_BACK_WRITEMASK, umath::is_flag_set(faceMask, StencilFaceFlags::BackBit) ? stencilWriteMask : 1);
	return GetContext().CheckResult();
}

bool prosper::GLCommandBuffer::RecordPipelineBarrier(const prosper::util::PipelineBarrierInfo &barrierInfo)
{
	// OpenGL does not require explicit pipeline barriers in the same way Vulkan does.
	// Insert appropriate glMemoryBarrier calls or no-op if synchronization is handled elsewhere.
	// glMemoryBarrier(GL_ALL_BARRIER_BITS);
	return true;
}

bool prosper::GLCommandBuffer::RecordSetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
	glPolygonOffset(depthBiasSlopeFactor, depthBiasConstantFactor);
	return true;
}
static void clear_image(prosper::GLContext &context, prosper::IImage &img, uint32_t layerId, uint32_t layerCount, uint32_t baseMipmap, uint32_t mipmapCount, const std::array<float, 4> &clearColor, std::optional<float> clearDepth, std::optional<float> clearStencil)
{
	GLint drawFboId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
	util::ScopeGuard sg {[drawFboId]() {
		// Restore previous bound framebuffer
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId);
	}};

	auto framebuffer = static_cast<prosper::GLImage &>(img).GetOrCreateFramebuffer(layerId, layerCount, baseMipmap, mipmapCount);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<prosper::GLFramebuffer &>(*framebuffer).GetGLFramebuffer());
	if(!clearDepth && !clearStencil) {
		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_STENCIL_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glClearColor(clearColor.at(0), clearColor.at(1), clearColor.at(2), clearColor.at(3));
		context.CheckResult();
		glClear(GL_COLOR_BUFFER_BIT);
		context.CheckResult();
		return;
	}

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);

	if(clearDepth.has_value()) {
		glDepthMask(GL_TRUE);
		GLboolean depthWritesEnabled;
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWritesEnabled);
		glDepthMask(GL_TRUE);

		glClearDepth(*clearDepth);
		glClear(GL_DEPTH_BUFFER_BIT);

		glDepthMask(depthWritesEnabled);
	}
	if(clearStencil.has_value()) {
		glStencilMask(GL_TRUE);
		GLboolean stencilWritesEnabled;
		glGetBooleanv(GL_STENCIL_WRITEMASK, &stencilWritesEnabled);
		glStencilMask(GL_TRUE);

		glClearStencil(*clearStencil);
		glClear(GL_STENCIL_BUFFER_BIT);

		glStencilMask(stencilWritesEnabled);
	}
}
bool prosper::GLCommandBuffer::RecordClearImage(IImage &img, ImageLayout layout, const std::array<float, 4> &clearColor, const prosper::util::ClearImageInfo &clearImageInfo)
{
	if(IsPrimary() == false)
		return false;
	clear_image(GetContext(), img, clearImageInfo.subresourceRange.baseArrayLayer, clearImageInfo.subresourceRange.layerCount, clearImageInfo.subresourceRange.baseMipLevel, clearImageInfo.subresourceRange.levelCount, clearColor, 0.f, false);
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
bool prosper::GLCommandBuffer::RecordClearImage(IImage &img, ImageLayout layout, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil, const util::ClearImageInfo &clearImageInfo)
{
	if(IsPrimary() == false)
		return false;
	clear_image(GetContext(), img, clearImageInfo.subresourceRange.baseArrayLayer, clearImageInfo.subresourceRange.layerCount, clearImageInfo.subresourceRange.baseMipLevel, clearImageInfo.subresourceRange.levelCount, {}, clearDepth, clearStencil);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordClearAttachment(IImage &img, const std::array<float, 4> &clearColor, uint32_t attId, uint32_t layerId, uint32_t layerCount)
{
	if(IsPrimary() == false)
		return false;
	clear_image(GetContext(), img, layerId, layerCount, 0, std::numeric_limits<uint32_t>::max(), clearColor, {}, {});
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordClearAttachment(IImage &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil, uint32_t layerId)
{
	if(IsPrimary() == false)
		return false;
	clear_image(GetContext(), img, layerId, 1, 0, std::numeric_limits<uint32_t>::max(), {}, clearDepth, clearStencil);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordUpdateBuffer(IBuffer &buffer, uint64_t offset, uint64_t size, const void *data)
{
	auto &glBuffer = buffer.GetAPITypeRef<GLBuffer>();
	glNamedBufferSubData(glBuffer.GetGLBuffer(), glBuffer.GetStartOffset() + offset, size, data);
	return GetContext().CheckResult();
}

bool prosper::GLCommandBuffer::RecordBindDescriptorSets(PipelineBindPoint bindPoint, prosper::Shader &shader, PipelineID shaderPipelineId, uint32_t firstSet, const std::vector<prosper::IDescriptorSet *> &descSets, const std::vector<uint32_t> dynamicOffsets)
{
	PipelineID pipelineId;
	if(shader.GetPipelineId(pipelineId, shaderPipelineId) == false)
		return false;
	auto &context = GetContext();
	auto &createInfo = *shader.GetPipelineCreateInfo(shaderPipelineId);
	auto &dsInfoItems = *createInfo.GetDsCreateInfoItems();
	auto numSets = descSets.size();
	for(auto i = decltype(numSets) {0u}; i < numSets; ++i) {
		auto setIdx = firstSet + i;
		auto &dsCreateInfo = dsInfoItems.at(setIdx);
		auto dsOffset = dynamicOffsets.empty() ? 0 : dynamicOffsets.at(i);
		auto *ds = descSets.at(i);
		UpdateLastUsageTimes(*ds);
		auto numBindings = ds->GetBindingCount();
		for(auto j = decltype(numBindings) {0u}; j < numBindings; ++j) {
			auto *binding = ds->GetBinding(j);
			if(binding == nullptr)
				continue; // No binding; Is this legal?
			auto bindingIdx = binding->GetBindingIndex();
			auto bindingPoint = context.ShaderPipelineDescSetBindingIndexToBindingPoint(pipelineId, setIdx, bindingIdx);
			if(bindingPoint.has_value() == false)
				continue;
			auto type = binding->GetType();
			switch(type) {
			case prosper::DescriptorSetBinding::Type::Texture:
				{
					std::optional<uint32_t> layer {};
					auto *tex = ds->GetBoundTexture(j, &layer);
					if(layer.has_value() && GetContext().IsValidationEnabled())
						GetContext().ValidationCallback(DebugMessageSeverityFlags::WarningBit, "Attempted to bind layer " + std::to_string(*layer) + " of texture '" + tex->GetDebugName() + "'! This is not allowed in OpenGL!");
					uint32_t activeTextureSlot = *bindingPoint;
					glBindTextureUnit(activeTextureSlot, tex ? static_cast<GLImage &>(tex->GetImage()).GetGLImage() : 0);
					auto *sampler = tex ? tex->GetSampler() : nullptr;
					glBindSampler(activeTextureSlot, sampler ? static_cast<GLSampler &>(*sampler).GetGLSampler() : 0);
					break;
				}
			case prosper::DescriptorSetBinding::Type::ArrayTexture:
				{
					auto numTextures = ds->GetBoundArrayTextureCount(j);
					for(auto i = decltype(numTextures) {0u}; i < numTextures; ++i) {
						auto *tex = ds->GetBoundArrayTexture(j, i);
						uint32_t activeTextureSlot = *bindingPoint + i;
						glBindTextureUnit(activeTextureSlot, tex ? static_cast<GLImage &>(tex->GetImage()).GetGLImage() : 0);
						auto *sampler = tex ? tex->GetSampler() : nullptr;
						glBindSampler(activeTextureSlot, sampler ? static_cast<GLSampler &>(*sampler).GetGLSampler() : 0);
					}
					break;
				}
			case prosper::DescriptorSetBinding::Type::UniformBuffer:
			case prosper::DescriptorSetBinding::Type::DynamicUniformBuffer:
				{
					DeviceSize offset, size;
					auto *buf = ds->GetBoundBuffer(j, &offset, &size);
					if(buf)
						glBindBufferRange(GL_UNIFORM_BUFFER, *bindingPoint, buf->GetAPITypeRef<GLBuffer>().GetGLBuffer(), buf->GetStartOffset() + offset + dsOffset, size);
					break;
				}
			case prosper::DescriptorSetBinding::Type::StorageBuffer:
				{
					DeviceSize offset, size;
					auto *buf = ds->GetBoundBuffer(j, &offset, &size);
					if(buf)
						glBindBufferRange(GL_SHADER_STORAGE_BUFFER, *bindingPoint, buf->GetAPITypeRef<GLBuffer>().GetGLBuffer(), buf->GetStartOffset() + offset + dsOffset, size);
					break;
				}
			}
		}
	}
	return GetContext().CheckResult();
}

bool prosper::GLCommandBuffer::RecordPushConstants(prosper::Shader &shader, PipelineID pipelineId, ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *data)
{
	// Push constants are small in size and usually don't change between render calls, so we
	// make sure the new data is actually different from what we already have (and if it's not, we can just skip the
	// update altogether).
	static std::array<uint8_t, IPrContext::MAX_COMMON_PUSH_CONSTANT_SIZE> pushConstantData {0};
	if(memcmp(pushConstantData.data() + offset, data, size) == 0)
		return true;
	memcpy(pushConstantData.data() + offset, data, size);

	auto &buf = GetContext().GetPushConstantBuffer();
	if(RecordUpdateBuffer(buf, offset, size, data) == false)
		return false;
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, buf.GetGLBuffer());
	return GetContext().CheckResult();
}

bool prosper::GLCommandBuffer::RecordBindDescriptorSets(PipelineBindPoint bindPoint, const IShaderPipelineLayout &pipelineLayout, uint32_t firstSet, const prosper::IDescriptorSet &descSet, uint32_t *optDynamicOffset)
{
	auto &glPipelineLayout = static_cast<const GLShaderPipelineLayout &>(pipelineLayout);
	auto *shader = glPipelineLayout.GetShader();
	if(shader == nullptr)
		return false;
	std::vector<uint32_t> dynamicOffsets;
	if(optDynamicOffset)
		dynamicOffsets.push_back(*optDynamicOffset);
	return RecordBindDescriptorSets(bindPoint, *shader, glPipelineLayout.GetPipelineIndex(), firstSet, {&const_cast<prosper::IDescriptorSet &>(descSet)}, dynamicOffsets);
}
bool prosper::GLCommandBuffer::RecordBindDescriptorSets(PipelineBindPoint bindPoint, const IShaderPipelineLayout &pipelineLayout, uint32_t firstSet, uint32_t numDescSets, const prosper::IDescriptorSet *const *descSets, uint32_t numDynamicOffsets, const uint32_t *dynamicOffsets)
{
	auto &glPipelineLayout = static_cast<const GLShaderPipelineLayout &>(pipelineLayout);
	auto *shader = glPipelineLayout.GetShader();
	if(shader == nullptr)
		return false;
	std::vector<prosper::IDescriptorSet *> vDescSets;
	vDescSets.reserve(numDescSets);
	for(auto i = decltype(numDescSets) {0u}; i < numDescSets; ++i)
		vDescSets.push_back(const_cast<prosper::IDescriptorSet *>(descSets[i]));
	std::vector<uint32_t> vDynamicOffsets;
	vDynamicOffsets.reserve(numDynamicOffsets);
	for(auto i = decltype(numDynamicOffsets) {0u}; i < numDynamicOffsets; ++i)
		vDynamicOffsets.push_back(dynamicOffsets[i]);
	return RecordBindDescriptorSets(bindPoint, *shader, glPipelineLayout.GetPipelineIndex(), firstSet, vDescSets, vDynamicOffsets);
}
bool prosper::GLCommandBuffer::RecordPushConstants(const IShaderPipelineLayout &pipelineLayout, ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *data)
{
	auto &glPipelineLayout = static_cast<const GLShaderPipelineLayout &>(pipelineLayout);
	auto *shader = glPipelineLayout.GetShader();
	if(shader == nullptr)
		return false;
	return RecordPushConstants(*shader, glPipelineLayout.GetPipelineIndex(), stageFlags, offset, size, data);
}

void prosper::GLCommandBuffer::ClearBoundPipeline()
{
	ICommandBuffer::ClearBoundPipeline();
	glBindVertexArray(0);
	auto numVertexAttribBindings = m_boundPipelineData.numVertexAttribBindings;
	for(auto i = decltype(numVertexAttribBindings) {0u}; i < numVertexAttribBindings; ++i)
		glDisableVertexAttribArray(i);
	m_boundPipelineData.pipelineId = {};
	m_boundPipelineData.shader = {};
	m_boundPipelineData.shaderPipelineId = {};
	m_boundPipelineData.nextActiveTextureIndex = 0;
	m_boundPipelineData.numVertexAttribBindings = 0;
}
std::optional<prosper::PipelineID> prosper::GLCommandBuffer::GetBoundPipelineId() const { return m_boundPipelineData.pipelineId; }
prosper::Shader *prosper::GLCommandBuffer::GetBoundShader() const { return m_boundPipelineData.shader.get(); }
bool prosper::GLCommandBuffer::DoRecordBindShaderPipeline(prosper::Shader &shader, PipelineID shaderPipelineId, PipelineID pipelineId)
{
	auto program = GetContext().GetPipelineProgram(pipelineId);
	if(program.has_value() == false)
		return false;
	glUseProgram(*program);

	if(shader.IsGraphicsShader()) {
		auto *createInfo = static_cast<prosper::GraphicsPipelineCreateInfo *>(shader.GetPipelineCreateInfo(shaderPipelineId));
		if(createInfo == nullptr)
			return false;
		bool blendingEnabled;
		prosper::BlendOp blendOpColor, blendOpAlpha;
		prosper::BlendFactor srcColorBlendFactor, dstColorBlendFactor, srcAlphaBlendFactor, dstAlphaBlendFactor;
		prosper::ColorComponentFlags channelWriteMask;
		auto res = createInfo->GetColorBlendAttachmentProperties(0 /* sub-pass id */, &blendingEnabled, &blendOpColor, &blendOpAlpha, &srcColorBlendFactor, &dstColorBlendFactor, &srcAlphaBlendFactor, &dstAlphaBlendFactor, &channelWriteMask);
		if(res && blendingEnabled) {
			glEnable(GL_BLEND);
			glBlendEquationSeparate(util::to_opengl_enum(blendOpColor), util::to_opengl_enum(blendOpAlpha));
			glBlendFuncSeparate(util::to_opengl_enum(srcColorBlendFactor), util::to_opengl_enum(dstColorBlendFactor), util::to_opengl_enum(srcAlphaBlendFactor), util::to_opengl_enum(dstAlphaBlendFactor));
			glColorMask(umath::is_flag_set(channelWriteMask, prosper::ColorComponentFlags::RBit), umath::is_flag_set(channelWriteMask, prosper::ColorComponentFlags::GBit), umath::is_flag_set(channelWriteMask, prosper::ColorComponentFlags::BBit),
			  umath::is_flag_set(channelWriteMask, prosper::ColorComponentFlags::ABit));
		}
		else
			glDisable(GL_BLEND);

		prosper::PolygonMode polygonMode;
		prosper::CullModeFlags cullModeFlags;
		prosper::FrontFace frontFace;
		float lineWidth;
		createInfo->GetRasterizationProperties(&polygonMode, &cullModeFlags, &frontFace, &lineWidth);
		switch(cullModeFlags) {
		case prosper::CullModeFlags::FrontAndBack:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT_AND_BACK);
			break;
		case prosper::CullModeFlags::BackBit:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;
		case prosper::CullModeFlags::FrontBit:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;
		default:
			glDisable(GL_CULL_FACE);
			break;
		}
		switch(frontFace) {
		case prosper::FrontFace::Clockwise:
			glFrontFace(GL_CW);
			break;
		case prosper::FrontFace::CounterClockwise:
			glFrontFace(GL_CCW);
			break;
		}
		glLineWidth(lineWidth);

		auto useScissor = false;
		auto numDynamicScissors = createInfo->GetDynamicScissorBoxesCount();
		if(numDynamicScissors > 0) {
			glEnable(GL_SCISSOR_TEST);
			useScissor = true;
		}
		else {
			int32_t scissorX, scissorY;
			uint32_t scissorW, scissorH;
			if(createInfo->GetScissorBoxesCount() > 0 && createInfo->GetScissorBoxProperties(0, &scissorX, &scissorY, &scissorW, &scissorH)) {
				glEnable(GL_SCISSOR_TEST);
				useScissor = true;
				SetScissor(scissorX, scissorY, scissorW, scissorH);
			}
			else
				glDisable(GL_SCISSOR_TEST);
		}

		auto isDepthBiasEnabled = false;
		createInfo->GetDepthBiasState(&isDepthBiasEnabled, nullptr, nullptr, nullptr);
		if(isDepthBiasEnabled)
			glEnable(GL_POLYGON_OFFSET_FILL);
		else
			glDisable(GL_POLYGON_OFFSET_FILL);

		auto customViewport = false;
		auto numDynamicViewports = createInfo->GetDynamicViewportsCount();
		if(numDynamicViewports == 0) {
			float viewportX, viewportY, viewportW, viewportH;
			float minDepth, maxDepth;
			if(createInfo->GetViewportCount() > 0 && createInfo->GetViewportProperties(0, &viewportX, &viewportY, &viewportW, &viewportH, &minDepth, &maxDepth)) {
				customViewport = true;
				SetViewport(viewportX, viewportY, viewportW, viewportH);
				glDepthRangef(minDepth, maxDepth);
			}
		}
		else
			customViewport = true;

		if(customViewport == false) {
			GLint viewportDims;
			glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &viewportDims); // TODO: Query this at context creation time
			SetViewport(0, 0, viewportDims, viewportDims);
			glDepthRangef(0.f, 1.f);
		}
		// ApplyViewport();
		// if(useScissor)
		// 	ApplyScissor(); // Scissor has to be applied AFTER viewport!

		bool isDepthTestEnabled;
		prosper::CompareOp depthCompareOp;
		createInfo->GetDepthTestState(&isDepthTestEnabled, &depthCompareOp);
		if(isDepthTestEnabled) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(util::to_opengl_enum(depthCompareOp));
		}
		else
			glDisable(GL_DEPTH_TEST);

		glDepthMask(createInfo->AreDepthWritesEnabled() ? GL_TRUE : GL_FALSE);
	}

	auto &buf = GetContext().GetPushConstantBuffer();
	// TODO
	//static_assert(false,"Bind push constant buffer");
	auto result = GetContext().CheckResult();
	if(result) {
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
void prosper::GLCommandBuffer::SetViewport(GLint x, GLint y, GLint w, GLint h)
{
	m_viewport = {x, y, w, h};
	ApplyViewport();
}
void prosper::GLCommandBuffer::SetScissor(GLint x, GLint y, GLint w, GLint h)
{
	m_scissor = {x, y, w, h};
	ApplyScissor();
}
void prosper::GLCommandBuffer::ApplyViewport()
{
	GLint vpX = m_viewport.at(0);
	GLint vpY = m_viewport.at(1);
	GLint vpW = m_viewport.at(2);
	GLint vpH = m_viewport.at(3);
	glViewport(vpX, vpY, vpW, vpH);
}
void prosper::GLCommandBuffer::ApplyScissor()
{
	GLint scX = m_scissor.at(0);
	GLint scY = m_scissor.at(1);
	GLint scW = m_scissor.at(2);
	GLint scH = m_scissor.at(3);
	if constexpr(SCISSOR_FLIP_Y)
		scY = GetContext().GetWindowHeight() - scY - scH;
	glScissor(scX, scY, scW, scH);
}
bool prosper::GLCommandBuffer::RecordSetViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y, float minDepth, float maxDepth)
{
	GLint vpX = x;
	GLint vpY = y;
	GLint vpW = width;
	GLint vpH = height;
	SetViewport(vpX, vpY, vpW, vpH);
	glDepthRangef(minDepth, maxDepth);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::RecordSetScissor(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
	GLint scX = x;
	GLint scY = y;
	GLint scW = width;
	GLint scH = height;
	SetScissor(scX, scY, scW, scH);
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

bool prosper::GLCommandBuffer::RecordPresentImage(IImage &img, IImage &swapchainImg, IFramebuffer &swapchainFramebuffer)
{
	//glBlitNamedFramebuffer(static_cast<GLImage &>(img).GetOrCreateFramebuffer(0, 1, 0, 1)->GetGLFramebuffer(), static_cast<GLFramebuffer &>(swapchainFramebuffer).GetGLFramebuffer(), 0, 0, img.GetWidth(), img.GetHeight(), 0, 0, swapchainImg.GetWidth(), swapchainImg.GetHeight(),
	//  GL_COLOR_BUFFER_BIT, GL_LINEAR);
	auto &context = static_cast<prosper::GLContext &>(GetContext());
	auto *shaderFlip = context.GetFlipShader();
	if(shaderFlip == nullptr)
		return false;

	GLint drawFboId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	static_cast<GLPrimaryCommandBuffer *>(this)->SetActiveRenderPassTarget(nullptr, 0, &swapchainImg, &swapchainFramebuffer);
	ShaderBindState bindState {*this};
	if(shaderFlip->RecordBeginDraw(bindState)) {
		SetViewport(0, 0, img.GetWidth(), img.GetHeight());
		glBindTextureUnit(0, static_cast<GLImage &>(img).GetGLImage());
		shaderFlip->RecordDraw(bindState, false /* flipHorizontally */, true /* flipVertically */);
		shaderFlip->RecordEndDraw(bindState);
	}

	static_cast<GLPrimaryCommandBuffer *>(this)->SetActiveRenderPassTarget(nullptr, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId);
	return true;
}

prosper::GLContext &prosper::GLCommandBuffer::GetContext() const { return static_cast<GLContext &>(ICommandBuffer::GetContext()); }

prosper::GLCommandBuffer::GLCommandBuffer(IPrContext &context, prosper::QueueFamilyType queueFamilyType) : ICommandBuffer {context, queueFamilyType} {}
bool prosper::GLCommandBuffer::DoRecordCopyBuffer(const prosper::util::BufferCopy &copyInfo, IBuffer &bufferSrc, IBuffer &bufferDst)
{
	glCopyNamedBufferSubData(bufferSrc.GetAPITypeRef<GLBuffer>().GetGLBuffer(), bufferDst.GetAPITypeRef<GLBuffer>().GetGLBuffer(), copyInfo.srcOffset, copyInfo.dstOffset, copyInfo.size);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::DoRecordCopyImage(const prosper::util::CopyInfo &copyInfo, IImage &imgSrc, IImage &imgDst, uint32_t w, uint32_t h)
{
	util::BlitInfo blitInfo {};
	blitInfo.extentsSrc = prosper::Extent2D {};
	blitInfo.extentsSrc->width = w;
	blitInfo.extentsSrc->height = h;

	blitInfo.extentsDst = prosper::Extent2D {};
	blitInfo.extentsDst->width = w;
	blitInfo.extentsDst->height = h;

	blitInfo.offsetSrc = {copyInfo.srcOffset.x, copyInfo.srcOffset.y};
	blitInfo.offsetDst = {copyInfo.dstOffset.x, copyInfo.dstOffset.y};

	blitInfo.srcSubresourceLayer = copyInfo.srcSubresource;
	blitInfo.dstSubresourceLayer = copyInfo.dstSubresource;

	std::array<Offset3D, 2> srcOffsets {};
	srcOffsets.at(0) = {blitInfo.offsetSrc.at(0), blitInfo.offsetSrc.at(1), 0};
	srcOffsets.at(1) = {static_cast<int32_t>(w), static_cast<int32_t>(h), 0};
	std::array<Offset3D, 2> dstOffsets {};
	dstOffsets.at(0) = {blitInfo.offsetDst.at(0), blitInfo.offsetDst.at(1), 0};
	dstOffsets.at(1) = {static_cast<int32_t>(w), static_cast<int32_t>(h), 0};
	return DoRecordBlitImage(blitInfo, imgSrc, imgDst, srcOffsets, dstOffsets);
}

bool prosper::GLCommandBuffer::DoRecordCopyBufferToImage(const prosper::util::BufferImageCopyInfo &copyInfo, IBuffer &bufferSrc, IImage &imgDst)
{
	static std::vector<uint8_t> imgData {};
	imgData.clear();
	auto &glImgDst = static_cast<GLImage &>(imgDst);

	Vector2i imgExtent {};
	if(copyInfo.imageExtent.has_value())
		imgExtent = *copyInfo.imageExtent;
	else
		imgExtent = {imgDst.GetWidth(copyInfo.mipLevel), imgDst.GetHeight(copyInfo.mipLevel)};

	if(util::is_compressed_format(imgDst.GetFormat()) && (imgExtent.x != imgDst.GetWidth(copyInfo.mipLevel) || imgExtent.y != imgDst.GetHeight(copyInfo.mipLevel)))
		return false;
	GLint size;
	if(util::is_compressed_format(imgDst.GetFormat()) == false)
		size = glImgDst.GetLayerSize(imgExtent.x, imgExtent.y);
	else {
		glGetTextureLevelParameteriv(glImgDst.GetGLImage(), copyInfo.mipLevel, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &size);
		auto numLayers = glImgDst.GetLayerCount();
		size /= numLayers;
	}
	imgData.resize(size);
	for(auto iLayer = copyInfo.baseArrayLayer; iLayer < (copyInfo.baseArrayLayer + copyInfo.layerCount); ++iLayer) {
		bufferSrc.Read(copyInfo.bufferOffset, imgData.size(), imgData.data());
		auto res = glImgDst.WriteImageData(copyInfo.imageOffset.x, copyInfo.imageOffset.y, imgExtent.x, imgExtent.y, iLayer, copyInfo.mipLevel, size, imgData.data());
		if(res == false)
			return false;
	}
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::DoRecordCopyImageToBuffer(const prosper::util::BufferImageCopyInfo &copyInfo, IImage &imgSrc, ImageLayout srcImageLayout, IBuffer &bufferDst)
{
	auto &glImgDst = static_cast<GLImage &>(imgSrc);
	auto format = imgSrc.GetFormat();

	Vector2i imgExtent {};
	if(copyInfo.imageExtent.has_value())
		imgExtent = *copyInfo.imageExtent;
	else
		imgExtent = {imgSrc.GetWidth(copyInfo.mipLevel), imgSrc.GetHeight(copyInfo.mipLevel)};

	static std::vector<uint8_t> pixelData {};
	if(util::is_compressed_format(format)) {
		GLint size;
		glGetTextureLevelParameteriv(glImgDst.GetGLImage(), copyInfo.mipLevel, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &size);

		pixelData.resize(size);
		glGetCompressedTextureSubImage(glImgDst.GetGLImage(), copyInfo.mipLevel, 0, 0, copyInfo.baseArrayLayer, imgExtent.x, imgExtent.y, copyInfo.layerCount, pixelData.size() * sizeof(pixelData.front()), pixelData.data());
	}
	else {
		auto size = imgExtent.x * imgExtent.y * util::get_byte_size(format) * copyInfo.layerCount;

		pixelData.resize(size);
		GLboolean normalized;
		auto imgFormatType = util::to_opengl_image_format_type(glImgDst.GetFormat(), normalized);
		glGetTextureSubImage(glImgDst.GetGLImage(), copyInfo.mipLevel, 0, 0, copyInfo.baseArrayLayer, imgExtent.x, imgExtent.y, copyInfo.layerCount, glImgDst.GetPixelDataFormat(), imgFormatType, pixelData.size() * sizeof(pixelData.front()), pixelData.data());
	}
	// TODO: Map buffer and write directly to mapped ptr instead of copying the data
	bufferDst.Write(copyInfo.bufferOffset, pixelData.size() * sizeof(pixelData.front()), pixelData.data());
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::DoRecordBlitImage(const util::BlitInfo &blitInfo, IImage &imgSrc, IImage &imgDst, const std::array<Offset3D, 2> &srcOffsets, const std::array<Offset3D, 2> &dstOffsets, std::optional<prosper::ImageAspectFlags> aspectFlags)
{
	if(util::is_compressed_format(imgDst.GetFormat()) || IsPrimary() == false)
		return false; // Can't blit into a compressed format
	auto framebufferDst = static_cast<GLImage &>(imgDst).GetOrCreateFramebuffer(blitInfo.dstSubresourceLayer.baseArrayLayer, blitInfo.dstSubresourceLayer.layerCount, blitInfo.dstSubresourceLayer.mipLevel, 1);
	if(util::is_compressed_format(imgSrc.GetFormat())) {
		if(srcOffsets.at(0).x > 0 || srcOffsets.at(0).y > 0 || dstOffsets.at(0).x > 0 || dstOffsets.at(0).y > 0 || srcOffsets.at(1).x != imgSrc.GetWidth() || srcOffsets.at(1).y != imgSrc.GetHeight() || dstOffsets.at(1).x != imgDst.GetWidth() || dstOffsets.at(1).y != imgDst.GetHeight())
			std::cout << "WARNING: Blit image offsets for compressed images currently not supported!" << std::endl;
		// Compressed textures can't be attached to a framebuffer (even for reading),
		// so we can't use glBlitFramebuffer to copy it. We'll have to use a shader to
		// do it instead.
		GLint drawFboId = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferDst->GetGLFramebuffer());
		static_cast<GLPrimaryCommandBuffer *>(this)->SetActiveRenderPassTarget(nullptr, blitInfo.dstSubresourceLayer.baseArrayLayer, &imgDst, framebufferDst.get());
		auto *shaderBlit = GetContext().GetBlitShader();
		ShaderBindState bindState {*this};
		if(shaderBlit->RecordBeginDraw(bindState)) {
			glBindTextureUnit(0, static_cast<GLImage &>(imgSrc).GetGLImage());
			shaderBlit->RecordDraw(bindState);
			shaderBlit->RecordEndDraw(bindState);
		}

		static_cast<GLPrimaryCommandBuffer *>(this)->SetActiveRenderPassTarget(nullptr, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId);
		return GetContext().CheckResult();
	}
	// These can affect blitting (despite the specifcation not making any mention about it)
	if(aspectFlags.has_value() && umath::is_flag_set(*aspectFlags, ImageAspectFlags::StencilBit))
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
	glDisable(GL_SCISSOR_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	auto framebufferSrc = static_cast<GLImage &>(imgSrc).GetOrCreateFramebuffer(blitInfo.srcSubresourceLayer.baseArrayLayer, blitInfo.srcSubresourceLayer.layerCount, blitInfo.srcSubresourceLayer.mipLevel, 1);
	glBlitNamedFramebuffer(framebufferSrc->GetGLFramebuffer(), framebufferDst->GetGLFramebuffer(), srcOffsets.at(0).x, srcOffsets.at(0).y, srcOffsets.at(0).x + srcOffsets.at(1).x, srcOffsets.at(0).y + srcOffsets.at(1).y, dstOffsets.at(0).x, dstOffsets.at(0).y,
	  dstOffsets.at(0).x + dstOffsets.at(1).x, dstOffsets.at(0).y + dstOffsets.at(1).y, static_cast<GLImage &>(imgSrc).GetBufferBit(), GL_LINEAR);
	return GetContext().CheckResult();
}
bool prosper::GLCommandBuffer::DoRecordResolveImage(IImage &imgSrc, IImage &imgDst, const prosper::util::ImageResolve &resolve)
{

	return false; // TODO
}

///////////////////

std::shared_ptr<prosper::GLCommandBufferPool> prosper::GLCommandBufferPool::Create(prosper::IPrContext &context, prosper::QueueFamilyType queueFamilyType) { return std::shared_ptr<GLCommandBufferPool> {new GLCommandBufferPool {context, queueFamilyType}}; }
std::shared_ptr<prosper::IPrimaryCommandBuffer> prosper::GLCommandBufferPool::AllocatePrimaryCommandBuffer() const { return GLPrimaryCommandBuffer::Create(GetContext(), GetQueueFamilyType()); }
std::shared_ptr<prosper::ISecondaryCommandBuffer> prosper::GLCommandBufferPool::AllocateSecondaryCommandBuffer() const { return GLSecondaryCommandBuffer::Create(GetContext(), GetQueueFamilyType()); }

prosper::GLCommandBufferPool::GLCommandBufferPool(prosper::IPrContext &context, prosper::QueueFamilyType queueFamilyType) : prosper::ICommandBufferPool {context, queueFamilyType} {}

//////////////

std::shared_ptr<prosper::GLPrimaryCommandBuffer> prosper::GLPrimaryCommandBuffer::Create(IPrContext &context, prosper::QueueFamilyType queueFamilyType)
{
	auto cmdBuf = std::shared_ptr<GLPrimaryCommandBuffer> {new GLPrimaryCommandBuffer {context, queueFamilyType}};
	cmdBuf->Initialize();
	return cmdBuf;
}
bool prosper::GLPrimaryCommandBuffer::IsPrimary() const { return true; }
prosper::GLPrimaryCommandBuffer::GLPrimaryCommandBuffer(IPrContext &context, prosper::QueueFamilyType queueFamilyType) : GLCommandBuffer {context, queueFamilyType}, ICommandBuffer {context, queueFamilyType} { m_apiTypePtr = this; }
bool prosper::GLPrimaryCommandBuffer::DoRecordBeginRenderPass(prosper::IImage &img, prosper::IRenderPass &rp, prosper::IFramebuffer &fb, uint32_t *layerId, const std::vector<prosper::ClearValue> &clearValues, RenderPassFlags renderPassFlags)
{
	glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLFramebuffer &>(fb).GetGLFramebuffer());
	auto &rpCreateInfo = rp.GetCreateInfo();
	for(auto attId = decltype(rpCreateInfo.attachments.size()) {0u}; attId < rpCreateInfo.attachments.size(); ++attId) {
		auto &attInfo = rpCreateInfo.attachments.at(attId);
		if(attInfo.loadOp != prosper::AttachmentLoadOp::Clear)
			continue;
		auto *imgView = fb.GetAttachment(attId);
		if(imgView == nullptr)
			return false;
		auto &attImg = imgView->GetImage();
		auto &clearVal = clearValues.at(attId);
		if(util::is_depth_format(attImg.GetFormat())) {
			if(layerId)
				RecordClearAttachment(attImg, clearVal.depthStencil.depth, *layerId);
			else
				RecordClearAttachment(attImg, clearVal.depthStencil.depth, {});
		}
		else {
			auto &clearCol = clearVal.color.float32;
			if(layerId)
				RecordClearAttachment(attImg, std::array<float, 4> {clearCol[0], clearCol[1], clearCol[2], clearCol[3]}, attId, *layerId, 1);
			else
				RecordClearAttachment(attImg, std::array<float, 4> {clearCol[0], clearCol[1], clearCol[2], clearCol[3]}, attId, 0u, 1);
		}
	}
	return dynamic_cast<GLContext &>(IPrimaryCommandBuffer::GetContext()).CheckResult();
}
bool prosper::GLPrimaryCommandBuffer::StartRecording(bool oneTimeSubmit, bool simultaneousUseAllowed) const { return IPrimaryCommandBuffer::StartRecording(oneTimeSubmit, simultaneousUseAllowed); }
bool prosper::GLPrimaryCommandBuffer::DoRecordEndRenderPass()
{
	//glBindFramebuffer(GL_FRAMEBUFFER,0);
	//return dynamic_cast<GLContext&>(IPrimaryCommandBuffer::GetContext()).CheckResult();
	return true;
}
bool prosper::GLPrimaryCommandBuffer::RecordNextSubPass()
{
	return false; // TODO
}

std::shared_ptr<prosper::GLSecondaryCommandBuffer> prosper::GLSecondaryCommandBuffer::Create(IPrContext &context, prosper::QueueFamilyType queueFamilyType)
{
	auto cmdBuf = std::shared_ptr<GLSecondaryCommandBuffer> {new GLSecondaryCommandBuffer {context, queueFamilyType}};
	cmdBuf->Initialize();
	return cmdBuf;
}
bool prosper::GLSecondaryCommandBuffer::IsSecondary() const { return true; }
prosper::GLSecondaryCommandBuffer::GLSecondaryCommandBuffer(IPrContext &context, prosper::QueueFamilyType queueFamilyType) : GLCommandBuffer {context, queueFamilyType}, ICommandBuffer {context, queueFamilyType}, ISecondaryCommandBuffer {context, queueFamilyType} { m_apiTypePtr = this; }
