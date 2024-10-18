/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
