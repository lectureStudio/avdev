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

#ifndef AVDEV_CORE_VIDEO_OUTPUT_STREAM_H_
#define AVDEV_CORE_VIDEO_OUTPUT_STREAM_H_

#include <memory>
#include "VideoStream.h"
#include "VideoSink.h"

namespace avdev
{
	using PVideoSink = std::shared_ptr<VideoSink>;


	class VideoOutputStream : public VideoStream
	{
		public:
			VideoOutputStream(PVideoSink sink);
			virtual ~VideoOutputStream() {};

		protected:
			void writeVideoFrame(const std::uint8_t * data, size_t length);

			PVideoSink sink;
	};


	using PVideoOutputStream = std::unique_ptr<VideoOutputStream>;
}

#endif