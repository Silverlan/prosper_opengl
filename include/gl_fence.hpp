/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PROSPER_GL_FENCE_HPP__
#define __PROSPER_GL_FENCE_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_fence.hpp>

namespace prosper
{
	class DLLPROSPER_GL GLFence
		: public prosper::IFence
	{
	public:
		static std::shared_ptr<IFence> Create(IPrContext &context,GLsync fence);

		virtual ~GLFence() override;
		virtual bool IsSet() const override;
		virtual bool Reset() const override;
	private:
		GLFence(IPrContext &context,GLsync fence);
		GLsync m_fence = nullptr;
	};
};

#endif
