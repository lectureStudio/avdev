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

#include "CoreAudioTypeConverter.h"
#include "AVdevException.h"

namespace avdev
{
	const AudioFormat CoreAudioTypeConverter::toAudioFormat(AudioStreamBasicDescription & desc)
	{
		SampleFormat format = SampleFormat::S16LE;
		unsigned sampleRate = static_cast<unsigned>(desc.mSampleRate);
		unsigned channels = static_cast<unsigned>(desc.mChannelsPerFrame);
		
		bool isBigEndian = (desc.mFormatFlags & kLinearPCMFormatFlagIsBigEndian);
		
		switch (desc.mFormatID) {
			case kAudioFormatLinearPCM:
				if (desc.mFormatFlags & kLinearPCMFormatFlagIsFloat) {
					switch (desc.mBitsPerChannel) {
						case 32:
							format = isBigEndian ? SampleFormat::FLOAT32BE : SampleFormat::FLOAT32LE;
							break;
						default:
							throw AVdevException("CoreAudio: Audio float format BitsPerChannel [%d] is not supported.", desc.mBitsPerChannel);
					}
				}
				else if (desc.mFormatFlags & kLinearPCMFormatFlagIsSignedInteger) {
					switch (desc.mBitsPerChannel) {
						case 16:
							format = isBigEndian ? SampleFormat::S16BE :  SampleFormat::S16LE;
							break;
						case 24:
							format = isBigEndian ? SampleFormat::S24BE :  SampleFormat::S24LE;
							break;
						case 32:
							format = isBigEndian ? SampleFormat::S32BE :  SampleFormat::S32LE;
							break;
						default:
							throw AVdevException("CoreAudio: Audio signed integer format BitsPerChannel [%d] is not supported.", desc.mBitsPerChannel);
					}
				}
				else {
					format = SampleFormat::U8;
				}
				break;
				
			case kAudioFormatALaw:
				format = SampleFormat::ALAW;
				break;
				
			case kAudioFormatULaw:
				format = SampleFormat::ULAW;
				break;
				
			default:
				throw AVdevException("CoreAudio: Audio format ID [%d] is not supported.", desc.mFormatID);
		}
		
		return AudioFormat(format, sampleRate, channels);
	}
	
	const AudioStreamBasicDescription CoreAudioTypeConverter::toStreamDescription(AudioFormat & format)
	{
		Float64 sampleRate = static_cast<Float64>(format.getSampleRate());
		UInt32 channels = static_cast<UInt32>(format.getChannels());
		AudioFormatID formatID = kAudioFormatLinearPCM;
		AudioFormatFlags formatFlags = kLinearPCMFormatFlagIsPacked;
		
		switch (format.getSampleFormat()) {
			case SampleFormat::U8:
				break;
				
			case SampleFormat::S16BE:
				formatFlags |= kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsBigEndian;
				break;
				
			case SampleFormat::S16LE:
				formatFlags |= kLinearPCMFormatFlagIsSignedInteger;
				break;
				
			case SampleFormat::S24BE:
				formatFlags |= kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsBigEndian;
				break;
				
			case SampleFormat::S24LE:
				formatFlags |= kLinearPCMFormatFlagIsSignedInteger;
				break;
				
			case SampleFormat::S32BE:
				formatFlags |= kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsBigEndian;
				break;
				
			case SampleFormat::S32LE:
				formatFlags |= kLinearPCMFormatFlagIsSignedInteger;
				break;
				
			case SampleFormat::FLOAT32BE:
				formatFlags |= kLinearPCMFormatFlagIsFloat | kLinearPCMFormatFlagIsBigEndian;
				break;
				
			case SampleFormat::FLOAT32LE:
				formatFlags |= kLinearPCMFormatFlagIsFloat;
				break;
				
			case SampleFormat::ALAW:
				formatID = kAudioFormatALaw;
				break;
				
			case SampleFormat::ULAW:
				formatID = kAudioFormatULaw;
				break;
		}
		
		AudioStreamBasicDescription desc = {0};
		desc.mSampleRate = sampleRate;
		desc.mFormatID = formatID;
		desc.mFormatFlags = formatFlags;
		desc.mBitsPerChannel = format.bitsPerSample();
		desc.mChannelsPerFrame = channels;
		desc.mFramesPerPacket = 1;  // Uncompressed audio
		desc.mBytesPerPacket = (desc.mBitsPerChannel * desc.mChannelsPerFrame) / 8;
		desc.mBytesPerFrame = desc.mBytesPerPacket;
		desc.mReserved = 0;
		
		return desc;
	}
}