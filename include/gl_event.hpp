// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PROSPER_GL_EVENT_HPP__
#define __PROSPER_GL_EVENT_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <prosper_event.hpp>

namespace prosper {
	class DLLPROSPER_GL GLEvent : public prosper::IEvent {
	  public:
		static std::shared_ptr<IEvent> Create(IPrContext &context);

		virtual ~GLEvent() override;
		virtual bool IsSet() const override;
	  private:
		GLEvent(IPrContext &context);
	};
};

#endif
