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

#ifndef AVDEV_COREAUDIO_STREAM_H_
#define AVDEV_COREAUDIO_STREAM_H_

#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>

#include "AudioFormat.h"
#include "Stream.h"
#include "MacOSHelper.h"

namespace avdev
{
	enum class CoreaAudioType
	{
		CoreAudioInput,
		CoreAudioOutput
	};
	
	class CoreAudioStream
	{
		public:
			CoreAudioStream(AudioDeviceID devID, AudioObjectPropertyScope scope);
			virtual ~CoreAudioStream();
			
			float getVolume();
			void setVolume(float volume);
			
			bool getMute();
			void setMute(bool mute);
			
			float getBufferLatency();
			
		protected:
			void open(CoreaAudioType type, AudioFormat format, unsigned latency);
			void start();
			void stop();
			void close();
		
			virtual void initDeviceBuffer(unsigned int bufferSize) = 0;
			virtual OSStatus processAudio(AudioUnitRenderActionFlags * actionFlags, const AudioTimeStamp * timeStamp, UInt32 busNumber, UInt32 numberFrames, AudioBufferList * data) = 0;

			const AudioDeviceID devID;
			const AudioObjectPropertyScope scope;
		
			AudioStreamBasicDescription deviceFormat;
			AudioStreamBasicDescription deviceIOFormat;
			AudioStreamBasicDescription converterFormat;
		
			AudioComponentInstance auHAL;
			AudioBufferList bufferList;
			AudioBufferList convBufferList;
			AudioConverterRef converter;
		
			ByteBuffer devBuffer;
			ByteBuffer convBuffer;

		private:
			bool canSetChannelVolume(unsigned channel);
		
			OSStatus getIOBufferFrameSizeRange(AudioUnit auHAL, UInt32 * outMinimum, UInt32 * outMaximum);
			OSStatus getIOBufferFrameSize(AudioUnit auHAL, UInt32 * outIOBufferFrameSize);
			OSStatus setIOBufferFrameSize(AudioUnit auHAL, UInt32 ioBufferFrameSize);
		
			void initIOBuffer(unsigned int bufferSize);
			void initResampler(CoreaAudioType type, AudioFormat format, UInt32 bufferFrameSize);
			void restart();
		
			static OSStatus ioProc(void * refCon, AudioUnitRenderActionFlags * actionFlags, const AudioTimeStamp * timeStamp, UInt32 busNumber, UInt32 numberFrames,  AudioBufferList * data);
			static OSStatus deviceListenerProc(AudioObjectID objectID, UInt32 numberAddresses, const AudioObjectPropertyAddress * addresses, void * clientData);
		
			// Parameters needed to restart the stream.
			StreamState state;
			CoreaAudioType type;
			AudioFormat format;
			unsigned latency;
	};
	
}

#endif