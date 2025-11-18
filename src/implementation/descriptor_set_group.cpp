// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.prosper.opengl;

import :descriptor_set_group;

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
