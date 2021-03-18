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

#include "CoreAudioOutputStream.h"
#include "Log.h"

namespace avdev
{
	CoreAudioOutputStream::CoreAudioOutputStream(AudioDeviceID devID, PAudioSink sink) :
		CoreAudioStream(devID, kAudioDevicePropertyScopeInput),
		AudioOutputStream(sink)
	{
	}
	
	float CoreAudioOutputStream::getVolume()
	{
		if (getState() == StreamState::CLOSED) {
			return AudioOutputStream::getVolume();
		}
		
		return CoreAudioStream::getVolume();
	}
	
	void CoreAudioOutputStream::setVolume(float volume)
	{
		AudioOutputStream::setVolume(volume);
		
		if (getState() != StreamState::CLOSED) {
			CoreAudioStream::setVolume(volume);
		}
	}
	
	bool CoreAudioOutputStream::getMute()
	{
		if (getState() == StreamState::CLOSED) {
			return AudioOutputStream::getMute();
		}
		
		return CoreAudioStream::getMute();
	}
	
	void CoreAudioOutputStream::setMute(bool mute)
	{
		AudioOutputStream::setMute(mute);
		
		if (getState() != StreamState::CLOSED) {
			CoreAudioStream::setMute(mute);
		}
	}
	
	void CoreAudioOutputStream::openInternal()
	{
		float volume = AudioOutputStream::getVolume();
		unsigned latency = AudioOutputStream::getBufferLatency();
		AudioFormat format = AudioOutputStream::getAudioFormat();
		
		CoreAudioStream::open(CoreaAudioType::CoreAudioOutput, format, latency);
		//CoreAudioStream::setVolume(volume);
	}
	
	void CoreAudioOutputStream::closeInternal()
	{
		CoreAudioStream::close();
		
		freeAudioBuffer();
	}
	
	void CoreAudioOutputStream::startInternal()
	{
		CoreAudioStream::start();
	}
	
	void CoreAudioOutputStream::stopInternal()
	{
		CoreAudioStream::stop();
	}
	
	void CoreAudioOutputStream::initDeviceBuffer(unsigned int bufferSize)
	{
		initAudioBuffer(bufferSize);
	}
	
	OSStatus CoreAudioOutputStream::processAudio(AudioUnitRenderActionFlags * actionFlags, const AudioTimeStamp * timeStamp, UInt32 busNumber, UInt32 numberFrames, AudioBufferList * data)
	{
		OSStatus status = AudioUnitRender(auHAL, actionFlags, timeStamp, busNumber, numberFrames, &bufferList);
		
		UInt32 frameBytes = numberFrames * deviceIOFormat.mBytesPerFrame;
		AudioBuffer * audioBuffer = bufferList.mBuffers;
		
		if (status == noErr) {
			std::uint8_t * samples = nullptr;
			
			// If a converter is present, then we have to resample audio.
			if (converter != nullptr) {
				convBufferList.mNumberBuffers = 1;
				
				AudioBuffer * convAudioBuffer = convBufferList.mBuffers;
				convAudioBuffer->mNumberChannels = converterFormat.mChannelsPerFrame;
				convAudioBuffer->mDataByteSize = static_cast<UInt32>(convBuffer.size());
				
				UInt32 convNumberPackets = convAudioBuffer->mDataByteSize / converterFormat.mBytesPerPacket;
				
				// Fill sample buffer and convert samples.
				OSStatus convStatus = AudioConverterFillComplexBuffer(converter, fillConverterBuffer, this, &convNumberPackets, &convBufferList, 0);
				
				frameBytes = convNumberPackets * converterFormat.mBytesPerFrame;
				
				if (convStatus == noErr) {
					samples = reinterpret_cast<std::uint8_t *>(convAudioBuffer->mData);
				}
				else {
					THROW_IF_FAILED(convStatus, "CoreAudio: Resample audio failed.");
				}
			}
			else {
				samples = reinterpret_cast<std::uint8_t *>(audioBuffer->mData);
			}
			
			if (samples != nullptr) {
				writeAudio(samples, frameBytes);
			}
		}
		else {
			THROW_IF_FAILED(status, "CoreAudio: Render input samples failed.");
		}

		return status;
	}
	
	OSStatus CoreAudioOutputStream::fillConverterBuffer(AudioConverterRef audioConverter, UInt32 * numberDataPackets, AudioBufferList * data, AudioStreamPacketDescription ** outDataPacketDescription, void * context)
	{
		CoreAudioOutputStream * stream = static_cast<CoreAudioOutputStream *>(context);
		
		// Tell the Audio Converter where it's source data is.
		data->mBuffers[0].mData = stream->bufferList.mBuffers[0].mData;
		// Tell the Audio Converter how much data is in each buffer.
		data->mBuffers[0].mDataByteSize = *numberDataPackets * stream->deviceIOFormat.mBytesPerPacket;
		
		return noErr;
	}
	
}