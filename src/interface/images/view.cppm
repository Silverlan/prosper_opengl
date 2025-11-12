// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.prosper.opengl:image.view;

export import pragma.prosper;

export namespace prosper {
	class PR_EXPORT GLImageView : public prosper::IImageView {
	  public:
		static std::shared_ptr<IImageView> Create(IPrContext &context, IImage &img, const util::ImageViewCreateInfo &createInfo, ImageViewType type, ImageAspectFlags aspectFlags);

		virtual ~GLImageView() override;
	  private:
		GLImageView(IPrContext &context, IImage &img, const util::ImageViewCreateInfo &createInfo, ImageViewType type, ImageAspectFlags aspectFlags);
	};
};
