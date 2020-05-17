/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "image/gl_image_view.hpp"

using namespace prosper;

std::shared_ptr<IImageView> GLImageView::Create(
	IPrContext &context,IImage &img,const prosper::util::ImageViewCreateInfo &createInfo,ImageViewType type,ImageAspectFlags aspectFlags
)
{
	return std::shared_ptr<GLImageView>{new GLImageView{context,img,createInfo,type,aspectFlags}};
}

GLImageView::~GLImageView() {}

GLImageView::GLImageView(
	IPrContext &context,IImage &img,const prosper::util::ImageViewCreateInfo &createInfo,ImageViewType type,ImageAspectFlags aspectFlags
)
	: IImageView{context,img,createInfo,type,aspectFlags}
{}
