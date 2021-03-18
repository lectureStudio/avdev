/*
 * Copyright 2016 Alex Andres
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "PictureFormat.h"
#include "AVdevException.h"

namespace avdev
{
	PictureFormat::PictureFormat(unsigned width, unsigned height, PixelFormat format) :
		width(width),
		height(height),
		pixelFormat(format)
	{
	}

	bool PictureFormat::operator== (const PictureFormat & other) const
	{
		return (width == other.width && height == other.height && pixelFormat == other.pixelFormat);
	}
	
	bool PictureFormat::operator!= (const PictureFormat & other) const
	{
		return !(*this == other);
	}

	void PictureFormat::setWidth(unsigned width)
	{
		this->width = width;
	}

	unsigned PictureFormat::getWidth() const
	{
		return width;
	}

	void PictureFormat::setHeight(unsigned height)
	{
		this->height = height;
	}

	unsigned PictureFormat::getHeight() const
	{
		return height;
	}

	void PictureFormat::setPixelFormat(PixelFormat format)
	{
		this->pixelFormat = format;
	}

	PixelFormat PictureFormat::getPixelFormat() const
	{
		return pixelFormat;
	}

	unsigned PictureFormat::getBytesPerPixel() const
	{
		switch (pixelFormat) {
			case PixelFormat::GREY:
				return 1;
			case PixelFormat::RGB555:
			case PixelFormat::RGB565:
				return 2;
			case PixelFormat::RGB24:
			case PixelFormat::BGR24:
				return 3;
			case PixelFormat::ARGB:
			case PixelFormat::BGR32:
			case PixelFormat::RGB32:
				return 4;
			default:
				throw AVdevException("Get bytes per pixel only supported by uncompressed formats.");
		}
	}

	std::string PictureFormat::toString()
	{
		std::string str = ToFccString(static_cast<uint32_t>(pixelFormat));
		str += ": " + std::to_string(width) + "x" + std::to_string(height);

		return str;
	}
}