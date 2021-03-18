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

#ifndef AVDEV_AVF_VIDEO_OUTPUT_STREAM_H_
#define AVDEV_AVF_VIDEO_OUTPUT_STREAM_H_

#include "AVFVideoStreamDelegate.h"
#include "AVFVideoFrameCallback.h"
#include "VideoOutputStream.h"

#include <AVFoundation/AVFoundation.h>

namespace avdev
{
	class AVFVideoOutputStream : public VideoOutputStream, AVFVideoFrameCallback
	{
		public:
			AVFVideoOutputStream(std::string devDescriptor, PVideoSink sink);
			virtual ~AVFVideoOutputStream();

			PictureFormat const& getPictureFormat() const;
			float getFrameRate() const;
		
			void onVideoFrame(const uint8_t * frameBuffer, size_t frameSize);

		protected:
			void openInternal();
			void closeInternal();
			void startInternal();
			void stopInternal();

		private:
			const std::string devDescriptor;
		
			AVFVideoStreamDelegate * streamDelegate;
	};
}

#endif