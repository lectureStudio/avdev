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

#ifndef AVDEV_CORE_PICTURE_FORMAT_H_
#define AVDEV_CORE_PICTURE_FORMAT_H_

#include <cstdint>
#include <string>

#define FOURCC(a,b,c,d) ( (uint32_t) (a << 0) + (b << 8) + (c << 16) + (d << 24) )

namespace avdev
{
	enum class PixelFormat
	{
		RGB332  = FOURCC('R', 'G', 'B', '8'),	/*  8  RGB-3-3-2      */
		RGB555  = FOURCC('R', '5', '5', '5'),	/* 16  RGB-5-5-5      */
		RGB565  = FOURCC('R', '5', '6', '5'),	/* 16  RGB-5-6-5      */
		BGR24   = FOURCC('B', 'G', 'R', '3'),	/* 24  BGR-8-8-8      */
		RGB24   = FOURCC('R', 'G', 'B', '3'),	/* 24  RGB-8-8-8      */
		BGR32   = FOURCC('B', 'G', 'R', '4'),	/* 32  BGR-8-8-8-8    */
		RGB32   = FOURCC('R', 'G', 'B', '4'),	/* 32  RGB-8-8-8-8    */
		ARGB    = FOURCC('A', 'R', 'G', 'B'),	/* 32  RGB-8-8-8-8    */
		GREY    = FOURCC('G', 'R', 'E', 'Y'),	/*  8  Greyscale      */
		YUY2    = FOURCC('Y', 'U', 'Y', '2'),	/*  8  4:2:2 Packed   */
		YV12    = FOURCC('Y', 'V', '1', '2'),	/*  8  4:2:0 Planar   */
		YVU410  = FOURCC('Y', 'V', 'U', '1'),	/*  9  YVU 4:1:0      */
		YUV410  = FOURCC('Y', 'U', 'V', '1'),	/*  9  YUV 4:1:0      */
		YVU420  = FOURCC('Y', 'V', 'U', '2'),	/* 12  YVU 4:2:0      */
		I420    = FOURCC('I', '4', '2', '0'),	/* 12  YUV 4:2:0      */
		YUYV    = FOURCC('Y', 'U', 'Y', 'V'),	/* 16  YUV 4:2:2      */
		UYVY    = FOURCC('U', 'Y', 'V', 'Y'),	/* 16  YUV 4:2:2      */
		YUV422P = FOURCC('I', '4', '2', '2'),	/* 16  YVU 422 planar */
		YUV411P = FOURCC('I', '4', '1', '1'),	/* 16  YVU 411 planar */
		Y41P    = FOURCC('Y', '4', '1', 'P'),	/* 12  YUV 4:1:1      */
		NV12    = FOURCC('N', 'V', '1', '2'),	/* 12  Y/CbCr 4:2:0   */
		NV21    = FOURCC('N', 'V', '2', '1'),	/* 12  Y/CrCb 4:2:0   */
		HI240   = FOURCC('H', 'I', '2', '4'),	/*  8  8-bit color    */
		JPEG    = FOURCC('J', 'P', 'E', 'G'),	/* JFIF JPEG          */
		MPEG    = FOURCC('M', 'P', 'E', 'G'),	/* MPEG               */
		MJPG    = FOURCC('M', 'J', 'P', 'G'),	/* Motion-JPEG        */
		DV      = FOURCC('D', 'V', ' ', ' '),	/* 1394               */
		WNVA    = FOURCC('W', 'N', 'V', 'A'),	/* Winnov hw compress */
		UNKNOWN = FOURCC('U', 'N', 'K', 'N')
	};

	inline std::string ToFccString(std::uint32_t fcc)
	{
		std::string str;
		str += fcc & 0xff;
		str += (fcc >> 8) & 0xff;
		str += (fcc >> 16) & 0xff;
		str += (fcc >> 24) & 0xff;

		return str;
	}

	inline std::string PixelFormatToString(PixelFormat format)
	{
		uint32_t fcc = static_cast<uint32_t>(format);
		return ToFccString(fcc);
	}

	class PictureFormat
	{
		public:
			PictureFormat(unsigned width, unsigned height, PixelFormat format);
			~PictureFormat() {};

			bool operator== (const PictureFormat & other) const;
			bool operator!= (const PictureFormat & other) const;

			unsigned getWidth() const;
			unsigned getHeight() const;

			void setWidth(unsigned width);
			void setHeight(unsigned height);

			PixelFormat getPixelFormat() const;
			void setPixelFormat(PixelFormat format);

			unsigned getBytesPerPixel() const;

			std::string toString();

		private:
			unsigned width;
			unsigned height;
			PixelFormat pixelFormat;
	};
}

#endif