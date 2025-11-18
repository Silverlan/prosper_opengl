// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "opengl_api.hpp"

export module pragma.prosper.opengl:fence;

export import pragma.prosper;

export namespace prosper {
	class PR_EXPORT GLFence : public prosper::IFence {
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
