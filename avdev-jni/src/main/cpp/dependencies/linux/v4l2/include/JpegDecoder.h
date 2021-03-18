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

#ifndef AVDEV_V4l2_JPEG_DECODER_H_
#define AVDEV_V4l2_JPEG_DECODER_H_

#include "Transform.h"

#include <setjmp.h>
#include <jpeglib.h>

namespace avdev
{
	class JpegDecoder : public Transform
	{
		public:
			~JpegDecoder() {};

			void transform(Bytes input, size_t inputSize, Bytes * output, size_t * outputSize);
			
		protected:
			static void onError(j_common_ptr info);
			static void onOutputMessage(j_common_ptr info);
		
		private:
			void addDHT(j_decompress_ptr decompress);
			void writeHuffTable(j_decompress_ptr decompress, JHUFF_TBL ** table, const UINT8 * bits, const UINT8 * values);
			
			ByteBuffer buffer;
	};
}

#endif