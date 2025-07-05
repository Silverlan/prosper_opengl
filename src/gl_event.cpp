// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
