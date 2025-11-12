// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "opengl_api.hpp"

export module pragma.prosper.opengl:image.sampler;

export import pragma.prosper;

export namespace prosper {
	class PR_EXPORT GLSampler : public prosper::ISampler {
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
