// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "image/gl_sampler.hpp"
#include "gl_util.hpp"
#include "gl_context.hpp"
#include "gl_api.hpp"

using namespace prosper;
std::shared_ptr<ISampler> GLSampler::Create(IPrContext &context, const prosper::util::SamplerCreateInfo &samplerCreateInfo, GLuint sampler) { return std::shared_ptr<GLSampler> {new GLSampler {context, samplerCreateInfo, sampler}}; }

GLSampler::GLSampler(IPrContext &context, const prosper::util::SamplerCreateInfo &samplerCreateInfo, GLuint sampler) : ISampler {context, samplerCreateInfo}, m_sampler {sampler} { Update(); }

GLSampler::~GLSampler() { glDeleteSamplers(1, &m_sampler); }

GLuint GLSampler::GetGLSampler() const { return m_sampler; }

bool GLSampler::DoUpdate()
{
	// See https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSamplerCreateInfo.html
	auto magFilter = GL_LINEAR_MIPMAP_LINEAR;
	switch(m_createInfo.magFilter) {
	case prosper::Filter::Linear:
		magFilter = GL_LINEAR;
		break;
	case prosper::Filter::Nearest:
		magFilter = GL_NEAREST;
		break;
	}
	glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, magFilter);

	auto minFilter = GL_LINEAR_MIPMAP_LINEAR;
	switch(m_createInfo.minFilter) {
	case prosper::Filter::Linear:
		switch(m_createInfo.mipmapMode) {
		case SamplerMipmapMode::Linear:
			minFilter = GL_LINEAR_MIPMAP_LINEAR;
			break;
		case SamplerMipmapMode::Nearest:
			minFilter = GL_LINEAR_MIPMAP_NEAREST;
			break;
		}
		break;
	case prosper::Filter::Nearest:
		switch(m_createInfo.mipmapMode) {
		case SamplerMipmapMode::Linear:
			minFilter = GL_NEAREST_MIPMAP_LINEAR;
			break;
		case SamplerMipmapMode::Nearest:
			minFilter = GL_NEAREST_MIPMAP_NEAREST;
			break;
		}
		break;
	}
	glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, minFilter);

	glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, util::to_opengl_enum(m_createInfo.addressModeU));
	glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, util::to_opengl_enum(m_createInfo.addressModeV));
	glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_R, util::to_opengl_enum(m_createInfo.addressModeW));

	glSamplerParameterf(m_sampler, GL_TEXTURE_MIN_LOD, m_createInfo.minLod);
	auto maxLod = m_createInfo.maxLod;
	if(maxLod == std::numeric_limits<decltype(maxLod)>::max())
		maxLod = 1000;
	glSamplerParameterf(m_sampler, GL_TEXTURE_MAX_LOD, maxLod);

	glSamplerParameteri(m_sampler, GL_TEXTURE_COMPARE_MODE, m_createInfo.compareEnable ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
	glSamplerParameteri(m_sampler, GL_TEXTURE_COMPARE_FUNC, m_createInfo.compareEnable ? util::to_opengl_enum(m_createInfo.compareOp) : GL_NEVER);

	glSamplerParameterf(m_sampler, GL_TEXTURE_LOD_BIAS, m_createInfo.mipLodBias);

	switch(m_createInfo.borderColor) {
	case BorderColor::FloatTransparentBlack:
		{
			std::array<float, 4> color {0.f, 0.f, 0.f, 0.f};
			glSamplerParameterfv(m_sampler, GL_TEXTURE_BORDER_COLOR, color.data());
			break;
		}
	case BorderColor::IntTransparentBlack:
		{
			std::array<int32_t, 4> color {0, 0, 0, 0};
			glSamplerParameteriv(m_sampler, GL_TEXTURE_BORDER_COLOR, color.data());
			break;
		}
	case BorderColor::FloatOpaqueBlack:
		{
			std::array<float, 4> color {0.f, 0.f, 0.f, 1.f};
			glSamplerParameterfv(m_sampler, GL_TEXTURE_BORDER_COLOR, color.data());
			break;
		}
	case BorderColor::IntOpaqueBlack:
		{
			std::array<int32_t, 4> color {0, 0, 0, 1};
			glSamplerParameteriv(m_sampler, GL_TEXTURE_BORDER_COLOR, color.data());
			break;
		}
	case BorderColor::FloatOpaqueWhite:
		{
			std::array<float, 4> color {1.f, 1.f, 1.f, 1.f};
			glSamplerParameterfv(m_sampler, GL_TEXTURE_BORDER_COLOR, color.data());
			break;
		}
	case BorderColor::IntOpaqueWhite:
		{
			std::array<int32_t, 4> color {1, 1, 1, 1};
			glSamplerParameteriv(m_sampler, GL_TEXTURE_BORDER_COLOR, color.data());
			break;
		}
	}

	auto maxAnisotropy = m_createInfo.maxAnisotropy;
	if(maxAnisotropy == std::numeric_limits<decltype(maxAnisotropy)>::max())
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);

	glSamplerParameterf(m_sampler, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
	return static_cast<GLContext &>(GetContext()).CheckResult();
}
