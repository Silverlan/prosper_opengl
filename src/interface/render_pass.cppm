// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.prosper.opengl:render_pass;

export import pragma.prosper;

export namespace prosper {
	class PR_EXPORT GLRenderPass : public prosper::IRenderPass {
	  public:
		static std::shared_ptr<IRenderPass> Create(IPrContext &context, const util::RenderPassCreateInfo &createInfo);

		virtual ~GLRenderPass() override;
	  private:
		GLRenderPass(IPrContext &context, const util::RenderPassCreateInfo &createInfo);
	};
};
