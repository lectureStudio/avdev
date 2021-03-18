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

#ifndef AVDEV_CORE_PIXEL_FORMAT_CONVERTER_H_
#define AVDEV_CORE_PIXEL_FORMAT_CONVERTER_H_

#include "PictureFormat.h"

#include <cstdint>
#include <map>
#include <memory>

namespace avdev
{
	class Converter
	{
		public:
			virtual ~Converter() {}

			virtual void convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength) = 0;

		protected:
			inline std::uint8_t clip(int color);
	};

	class YUYV_RGB24 : public Converter
	{
		public:
			void convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength);
	};

	class YVYU_RGB24 : public Converter
	{
		public:
			void convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength);
	};

	class UYVY_RGB24 : public Converter
	{
		public:
			void convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength);
	};

	class RGB565_RGB24 : public Converter
	{
		public:
			void convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength);
	};

	class PixelFormatConverter : public Converter
	{
		public:
			PixelFormatConverter();

			void init(PictureFormat srcFormat, PictureFormat dstFormat);

			void convert(const std::uint8_t * src, std::uint8_t * dest, int frameLength);

			PictureFormat const& getOutputFormat() const;

		private:
			std::map<std::pair<PixelFormat, PixelFormat>, std::shared_ptr<Converter>> convMap;
			std::shared_ptr<Converter> converter;
			PictureFormat dstFormat;
	};
}

#endif