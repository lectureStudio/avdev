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

#ifndef AVDEV_CORE_IMAGE_UTILS_H_
#define AVDEV_CORE_IMAGE_UTILS_H_

#include <cstdlib>
#include <cstdint>
#include <cstring>

namespace avdev
{
	namespace ImageUtils
	{
		static void flipVertically(std::uint8_t * pixels, const size_t width, const size_t height, const unsigned bytesPerPixel)
		{
			const size_t stride = width * bytesPerPixel;
			std::uint8_t * row = static_cast<std::uint8_t *>(std::malloc(stride));
			std::uint8_t * low = pixels;
			std::uint8_t * high = &pixels[(height - 1) * stride];

			for (; low < high; low += stride, high -= stride) {
				std::memcpy(row, low, stride);
				std::memcpy(low, high, stride);
				std::memcpy(high, row, stride);
			}
			std::free(row);
		}
	}
}

#endif