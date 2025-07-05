// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PROSPER_GL_FENCE_HPP__
#define __PROSPER_GL_FENCE_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_fence.hpp>

namespace prosper {
	class DLLPROSPER_GL GLFence : public prosper::IFence {
	  public:
		static std::shared_ptr<IFence> Create(IPrContext &context);

		virtual ~GLFence() override;
		virtual bool IsSet() const override;
		virtual bool Reset() const override;
		void Wait();
		virtual const void *GetInternalHandle() const override { return m_fence; }
	  private:
		void Clear() const;
		GLFence(IPrContext &context);
		mutable GLsync m_fence = nullptr;
	};
};

#endif
