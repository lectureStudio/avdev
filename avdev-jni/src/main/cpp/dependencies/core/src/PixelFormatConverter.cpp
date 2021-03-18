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

#include "PixelFormatConverter.h"
#include "AVdevException.h"

namespace avdev
{
	PixelFormatConverter::PixelFormatConverter() :
		dstFormat(PictureFormat(0, 0, PixelFormat::UNKNOWN))
	{
		convMap[{PixelFormat::YUYV, PixelFormat::RGB24}] = std::make_shared<YUYV_RGB24>();
		//convMap[{PixelFormat::YVYU, PixelFormat::RGB24}] = std::make_shared<YVYU_RGB24>();
		convMap[{PixelFormat::UYVY, PixelFormat::RGB24}] = std::make_shared<UYVY_RGB24>();
		convMap[{PixelFormat::RGB565, PixelFormat::RGB24}] = std::make_shared<RGB565_RGB24>();
	}
	
	void PixelFormatConverter::init(PictureFormat srcFormat, PictureFormat dstFormat)
	{
		this->dstFormat = dstFormat;
		
		converter = convMap[{srcFormat.getPixelFormat(), dstFormat.getPixelFormat()}];
		
		if (converter == nullptr) {
			std::string src = ToFccString(static_cast<uint32_t>(srcFormat.getPixelFormat()));
			std::string dst = ToFccString(static_cast<uint32_t>(dstFormat.getPixelFormat()));
			
			throw AVdevException("Pixel format conversion not implemented: [%s] -> [%s]", src.c_str(), dst.c_str());
		}
	}
	
	void PixelFormatConverter::convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength)
	{
		if (converter == nullptr) {
			throw AVdevException("Not initialized. Call ::init() first.");
		}
		
		converter->convert(src, dest, frameLength);
	}
	
	PictureFormat const& PixelFormatConverter::getOutputFormat() const
	{
		return dstFormat;
	}
	
	inline std::uint8_t Converter::clip(int color)
	{
		return (color > 0xFF) ? 0xFF : ((color < 0) ? 0 : color);
	}

	void YUYV_RGB24::convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength)
	{
		for (int j = 0; j + 1 < frameLength; j += 2) {
			int u = src[1];
			int v = src[3];
			int u1 = (((u - 128) << 7) + (u - 128)) >> 6;
			int rg = (((u - 128) << 1) + (u - 128) + ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
			int v1 = (((v - 128) << 1) + (v - 128)) >> 1;

			*dest++ = clip(src[0] + v1);
			*dest++ = clip(src[0] - rg);
			*dest++ = clip(src[0] + u1);

			*dest++ = clip(src[2] + v1);
			*dest++ = clip(src[2] - rg);
			*dest++ = clip(src[2] + u1);
				
			src += 4;
		}
	}
		
	void YVYU_RGB24::convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength)
	{
		for (int j = 0; j < frameLength; j += 2) {
			int u = src[3];
			int v = src[1];
			int u1 = (((u - 128) << 7) + (u - 128)) >> 6;
			int rg = (((u - 128) << 1) + (u - 128) + ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
			int v1 = (((v - 128) << 1) + (v - 128)) >> 1;
		
			*dest++ = clip(src[0] + v1);
			*dest++ = clip(src[0] - rg);
			*dest++ = clip(src[0] + u1);
		
			*dest++ = clip(src[2] + v1);
			*dest++ = clip(src[2] - rg);
			*dest++ = clip(src[2] + u1);
					
			src += 4;
		}
	}
		
	void UYVY_RGB24::convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength)
	{
		for (int j = 0; j < frameLength; j += 2) {
			int u = src[0];
			int v = src[2];
			int u1 = (((u - 128) << 7) + (u - 128)) >> 6;
			int rg = (((u - 128) << 1) + (u - 128) + ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
			int v1 = (((v - 128) << 1) + (v - 128)) >> 1;
		
			*dest++ = clip(src[1] + v1);
			*dest++ = clip(src[1] - rg);
			*dest++ = clip(src[1] + u1);
		
			*dest++ = clip(src[3] + v1);
			*dest++ = clip(src[3] - rg);
			*dest++ = clip(src[3] + u1);
					
			src += 4;
		}
	}
		
	void RGB565_RGB24::convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength)
	{
		for (int j = 0; j < frameLength; j++) {
			unsigned short tmp = *(unsigned short *)src;
		
			/* Original format: rrrrrggg gggbbbbb */
			*dest++ = 0xf8 & (tmp >> 8);
			*dest++ = 0xfc & (tmp >> 3);
			*dest++ = 0xf8 & (tmp << 3);
		
			src += 2;
		}
	}
}