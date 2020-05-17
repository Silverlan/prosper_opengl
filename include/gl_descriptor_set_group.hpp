/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PROSPER_GL_DESCRIPTOR_SET_GROUP_HPP__
#define __PROSPER_GL_DESCRIPTOR_SET_GROUP_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_descriptor_set_group.hpp>

namespace prosper
{
	class DLLPROSPER_GL GLDescriptorSetGroup
		: public IDescriptorSetGroup
	{
	public:
		static std::shared_ptr<GLDescriptorSetGroup> Create(IPrContext &context,const DescriptorSetCreateInfo &createInfo);
		virtual ~GLDescriptorSetGroup() override;
	protected:
		GLDescriptorSetGroup(IPrContext &context,const DescriptorSetCreateInfo &createInfo);
	};
};

#endif
