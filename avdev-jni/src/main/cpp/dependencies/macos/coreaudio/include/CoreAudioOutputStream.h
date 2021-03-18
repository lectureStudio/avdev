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

#ifndef AVDEV_COREAUDIO_OUTPUT_STREAM_H_
#define AVDEV_COREAUDIO_OUTPUT_STREAM_H_

#include <CoreAudio/CoreAudio.h>
#include "AudioOutputStream.h"
#include "CoreAudioStream.h"

namespace avdev
{
	class CoreAudioOutputStream : public CoreAudioStream, public AudioOutputStream
	{
		public:
			CoreAudioOutputStream(AudioDeviceID devID, PAudioSink sink);
			virtual ~CoreAudioOutputStream() {};
			
			float getVolume();
			void setVolume(float volume);
			
			bool getMute();
			void setMute(bool mute);
			
		protected:
			void openInternal();
			void closeInternal();
			void startInternal();
			void stopInternal();
			
			void initDeviceBuffer(unsigned int bufferSize);
			OSStatus processAudio(AudioUnitRenderActionFlags * actionFlags, const AudioTimeStamp * timeStamp, UInt32 busNumber, UInt32 numberFrames, AudioBufferList * data);
		
		private:
			static OSStatus fillConverterBuffer(AudioConverterRef audioConverter, UInt32 * numberDataPackets, AudioBufferList * data, AudioStreamPacketDescription ** outDataPacketDescription, void * context);
	};
}

#endif