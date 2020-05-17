/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_descriptor_set_group.hpp"

using namespace prosper;

std::shared_ptr<GLDescriptorSetGroup> GLDescriptorSetGroup::Create(IPrContext &context,const DescriptorSetCreateInfo &createInfo)
{
	return std::shared_ptr<GLDescriptorSetGroup>{new GLDescriptorSetGroup{context,createInfo}};
}
GLDescriptorSetGroup::~GLDescriptorSetGroup()
{}
GLDescriptorSetGroup::GLDescriptorSetGroup(IPrContext &context,const DescriptorSetCreateInfo &createInfo)
	: IDescriptorSetGroup{context,createInfo}
{}