/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_event.hpp"

using namespace prosper;

std::shared_ptr<IEvent> GLEvent::Create(IPrContext &context) { return std::shared_ptr<GLEvent> {new GLEvent {context}}; }

GLEvent::~GLEvent() {}
GLEvent::GLEvent(IPrContext &context) : IEvent {context} {}
bool GLEvent::IsSet() const
{
	// TODO
	return false;
}
