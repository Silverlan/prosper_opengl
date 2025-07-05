// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PROSPER_GL_SAMPLER_HPP__
#define __PROSPER_GL_SAMPLER_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <image/prosper_sampler.hpp>

namespace prosper {
	class DLLPROSPER_GL GLSampler : public prosper::ISampler {
	  public:
		static std::shared_ptr<ISampler> Create(IPrContext &context, const util::SamplerCreateInfo &samplerCreateInfo, GLuint sampler);

		GLuint GetGLSampler() const;
		virtual ~GLSampler() override;
		virtual const void *GetInternalHandle() const override { return reinterpret_cast<void *>(m_sampler); }
	  private:
		GLSampler(IPrContext &context, const util::SamplerCreateInfo &samplerCreateInfo, GLuint sampler);
		virtual bool DoUpdate() override;
		GLuint m_sampler = GL_INVALID_VALUE;
	};
};

#endif
