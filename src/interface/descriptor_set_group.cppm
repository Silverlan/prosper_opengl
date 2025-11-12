// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.prosper.opengl:descriptor_set_group;

export import pragma.prosper;

export namespace prosper {
	class PR_EXPORT GLDescriptorSetGroup : public IDescriptorSetGroup {
	  public:
		static std::shared_ptr<GLDescriptorSetGroup> Create(IPrContext &context, const DescriptorSetCreateInfo &createInfo);
		virtual ~GLDescriptorSetGroup() override;
	  protected:
		GLDescriptorSetGroup(IPrContext &context, const DescriptorSetCreateInfo &createInfo);
	};

	class PR_EXPORT GLDescriptorSet : public IDescriptorSet {
	  public:
		GLDescriptorSet(GLDescriptorSetGroup &dsg);

		virtual bool Update() override;
	  protected:
		//TODO: implement these
		virtual bool DoSetBindingStorageImage(prosper::Texture &texture, uint32_t bindingIdx, const std::optional<uint32_t> &layerId) override { return true; }
		virtual bool DoSetBindingTexture(prosper::Texture &texture, uint32_t bindingIdx, const std::optional<uint32_t> &layerId) override { return true; }
		virtual bool DoSetBindingArrayTexture(prosper::Texture &texture, uint32_t bindingIdx, uint32_t arrayIndex, const std::optional<uint32_t> &layerId) override { return true; }
		virtual bool DoSetBindingUniformBuffer(prosper::IBuffer &buffer, uint32_t bindingIdx, uint64_t startOffset, uint64_t size) override { return true; }
		virtual bool DoSetBindingDynamicUniformBuffer(prosper::IBuffer &buffer, uint32_t bindingIdx, uint64_t startOffset, uint64_t size) override { return true; }
		virtual bool DoSetBindingStorageBuffer(prosper::IBuffer &buffer, uint32_t bindingIdx, uint64_t startOffset, uint64_t size) override { return true; }
		virtual bool DoSetBindingDynamicStorageBuffer(prosper::IBuffer &buffer, uint32_t bindingIdx, uint64_t startOffset, uint64_t size) override { return true; };
	};
};
