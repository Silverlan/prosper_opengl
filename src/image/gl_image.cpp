/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "image/gl_image.hpp"

using namespace prosper;

std::shared_ptr<IImage> GLImage::Create(IPrContext &context,const prosper::util::ImageCreateInfo &createInfo)
{
	return std::shared_ptr<GLImage>{new GLImage{context,createInfo}};
}
GLImage::GLImage(IPrContext &context,const prosper::util::ImageCreateInfo &createInfo)
	: IImage{context,createInfo}
{}
GLImage::~GLImage()
{
	glDeleteTextures(1,&m_image);
}
std::optional<prosper::util::SubresourceLayout> GLImage::GetSubresourceLayout(uint32_t layerId,uint32_t mipMapIdx)
{
	prosper::util::SubresourceLayout layout {};
	// TODO
#if 0
	struct DLLPROSPER SubresourceLayout
	{
		DeviceSize offset;
		DeviceSize size;
		DeviceSize row_pitch;
		DeviceSize array_pitch;
		DeviceSize depth_pitch;
	};
#endif
	return layout; // TODO
}
DeviceSize GLImage::GetAlignment() const
{
	return 0; // TODO
}
bool GLImage::Map(DeviceSize offset,DeviceSize size,void **outPtr)
{
	return false; // TODO
}

bool GLImage::DoSetMemoryBuffer(IBuffer &buffer)
{
	return false; // TODO
}
