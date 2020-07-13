/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PROSPER_GL_SAMPLER_HPP__
#define __PROSPER_GL_SAMPLER_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <image/prosper_sampler.hpp>

namespace prosper
{
	class DLLPROSPER_GL GLSampler
		: public prosper::ISampler
	{
	public:
		static std::shared_ptr<ISampler> Create(IPrContext &context,const util::SamplerCreateInfo &samplerCreateInfo,GLuint sampler);

		GLuint GetGLSampler() const;
		virtual ~GLSampler() override;
	private:
		GLSampler(IPrContext &context,const util::SamplerCreateInfo &samplerCreateInfo,GLuint sampler);
		virtual bool DoUpdate() override;
		GLuint m_sampler = GL_INVALID_VALUE;
	};
};

#endif
