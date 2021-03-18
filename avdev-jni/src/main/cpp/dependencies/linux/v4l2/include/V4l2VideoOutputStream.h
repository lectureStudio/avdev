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

#ifndef AVDEV_V4l2_VIDEO_OUTPUT_STREAM_H_
#define AVDEV_V4l2_VIDEO_OUTPUT_STREAM_H_

#include "avdev-v4l2.h"
#include "Thread.h"
#include "VideoOutputStream.h"
#include "JpegDecoder.h"
#include "PixelFormatConverter.h"
#include <vector>

namespace avdev
{
	using V4l2Buffer = std::pair<void *, size_t>;
	using V4l2Buffers = std::vector<V4l2Buffer>;


	class V4l2VideoOutputStream : public VideoOutputStream, public Thread
	{
		public:
			V4l2VideoOutputStream(std::string devDescriptor, PVideoSink sink);
			virtual ~V4l2VideoOutputStream();

			PictureFormat getPictureFormat();
			float getFrameRate();

		protected:
			void openInternal();
			void closeInternal();
			void startInternal();
			void stopInternal();

			void run();
			int captureFrame();

			void initBuffer(unsigned int pictureSize);

			std::uint8_t * getBuffer(std::uint8_t index);
			size_t getBufferSize(std::uint8_t index);

		private:
            std::shared_ptr<avdev::PixelFormatConverter> converter;

			/* Two buffers should be sufficient. Back + Front buffer.*/
			const int maxBuffers;

			std::string devDescriptor;

			v4l2::IOMethod ioMethod;
			int v4l2_fd;

			V4l2Buffers buffers;

			JpegDecoder jpegDecoder;
	};
}

#endif