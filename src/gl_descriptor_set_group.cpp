/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_descriptor_set_group.hpp"

using namespace prosper;

std::shared_ptr<GLDescriptorSetGroup> GLDescriptorSetGroup::Create(IPrContext &context, const DescriptorSetCreateInfo &createInfo) { return std::shared_ptr<GLDescriptorSetGroup> {new GLDescriptorSetGroup {context, createInfo}}; }
GLDescriptorSetGroup::~GLDescriptorSetGroup() {}
GLDescriptorSetGroup::GLDescriptorSetGroup(IPrContext &context, const DescriptorSetCreateInfo &createInfo) : IDescriptorSetGroup {context, createInfo}
{
	auto numSets = 1u; // TODO
	m_descriptorSets.resize(numSets);

	for(auto i = decltype(m_descriptorSets.size()) {0u}; i < m_descriptorSets.size(); ++i) {
		m_descriptorSets.at(i) = std::shared_ptr<GLDescriptorSet> {new GLDescriptorSet {*this}, [](GLDescriptorSet *ds) {
			                                                           // ds->OnRelease();
			                                                           delete ds;
		                                                           }};
	}
}

///////////////////////

GLDescriptorSet::GLDescriptorSet(GLDescriptorSetGroup &dsg) : IDescriptorSet {dsg} { m_apiTypePtr = static_cast<GLDescriptorSet *>(this); }
/*struct DescriptorSetBinding
{
	enum class Type : uint8_t
	{
		StorageImage = 0,
		Texture,
		ArrayTexture,
		UniformBuffer,
		DynamicUniformBuffer,
		StorageBuffer
	};
	virtual ~DescriptorSetBinding()=default;
};*/
struct DescriptorSetBindingStorageImage : public DescriptorSetBinding {};
bool GLDescriptorSet::Update() { return true; }
