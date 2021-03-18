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

#ifndef AVDEV_MF_VIDEO_OUTPUT_STREAM_H_
#define AVDEV_MF_VIDEO_OUTPUT_STREAM_H_

#include "VideoOutputStream.h"
#include "MFOutputStream.h"

namespace avdev
{
	class MFVideoOutputStream : public VideoOutputStream, MFOutputStream
	{
		public:
			MFVideoOutputStream(std::string symbolicLink, PVideoSink sink);
			~MFVideoOutputStream();

			void setPictureFormat(PictureFormat format);
			void setFrameRate(float frameRate);

		protected:
			void openInternal();
			void closeInternal();
			void startInternal();
			void stopInternal();

			void onTopologyReady();

			void processSample(const BYTE * sampleBuffer, DWORD & sampleSize);

		private:
			std::string symbolicLink;
	};
}

#endif