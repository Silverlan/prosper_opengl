// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PROSPER_GL_IMAGE_VIEW_HPP__
#define __PROSPER_GL_IMAGE_VIEW_HPP__

#include <prosper_opengl_definitions.hpp>
#include <prosper_includes.hpp>
#include <image/prosper_image_view.hpp>

namespace prosper {
	class DLLPROSPER_GL GLImageView : public prosper::IImageView {
	  public:
		static std::shared_ptr<IImageView> Create(IPrContext &context, IImage &img, const util::ImageViewCreateInfo &createInfo, ImageViewType type, ImageAspectFlags aspectFlags);

		virtual ~GLImageView() override;
	  private:
		GLImageView(IPrContext &context, IImage &img, const util::ImageViewCreateInfo &createInfo, ImageViewType type, ImageAspectFlags aspectFlags);
	};
};

#endif
