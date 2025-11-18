// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.prosper.opengl;

import :image.view;

using namespace prosper;

std::shared_ptr<IImageView> GLImageView::Create(IPrContext &context, IImage &img, const prosper::util::ImageViewCreateInfo &createInfo, ImageViewType type, ImageAspectFlags aspectFlags) { return std::shared_ptr<GLImageView> {new GLImageView {context, img, createInfo, type, aspectFlags}}; }

GLImageView::~GLImageView() {}

GLImageView::GLImageView(IPrContext &context, IImage &img, const prosper::util::ImageViewCreateInfo &createInfo, ImageViewType type, ImageAspectFlags aspectFlags) : IImageView {context, img, createInfo, type, aspectFlags} {}
