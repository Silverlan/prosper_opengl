/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
