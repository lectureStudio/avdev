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

#include "CoreAudioInputStream.h"
#include <algorithm>

namespace avdev
{
	CoreAudioInputStream::CoreAudioInputStream(AudioDeviceID devID, PAudioSource source) :
		CoreAudioStream(devID, kAudioDevicePropertyScopeOutput),
		AudioInputStream(source)
	{
	}
	
	float CoreAudioInputStream::getVolume()
	{
		if (getState() == StreamState::CLOSED) {
			return AudioInputStream::getVolume();
		}
		
		return CoreAudioStream::getVolume();
	}
	
	void CoreAudioInputStream::setVolume(float volume)
	{
		AudioInputStream::setVolume(volume);
		
		if (getState() != StreamState::CLOSED) {
			CoreAudioStream::setVolume(volume);
		}
	}
	
	bool CoreAudioInputStream::getMute()
	{
		if (getState() == StreamState::CLOSED) {
			return AudioInputStream::getMute();
		}
		
		return CoreAudioStream::getMute();
	}
	
	void CoreAudioInputStream::setMute(bool mute)
	{
		AudioInputStream::setMute(mute);
		
		if (getState() != StreamState::CLOSED) {
			CoreAudioStream::setMute(mute);
		}
	}
	
	void CoreAudioInputStream::openInternal()
	{
		float volume = AudioInputStream::getVolume();
		unsigned latency = AudioInputStream::getBufferLatency();
		AudioFormat format = AudioInputStream::getAudioFormat();
		
		CoreAudioStream::open(CoreaAudioType::CoreAudioInput, format, latency);
		//CoreAudioStream::setVolume(volume);
	}
	
	void CoreAudioInputStream::closeInternal()
	{
		CoreAudioStream::close();
		
		freeAudioBuffer();
	}
	
	void CoreAudioInputStream::startInternal()
	{
		CoreAudioStream::start();
	}
	
	void CoreAudioInputStream::stopInternal()
	{
		CoreAudioStream::stop();
	}
	
	void CoreAudioInputStream::initDeviceBuffer(unsigned int bufferSize)
	{
		initAudioBuffer(bufferSize * 2);
	}
	
	OSStatus CoreAudioInputStream::processAudio(AudioUnitRenderActionFlags * actionFlags, const AudioTimeStamp * timeStamp, UInt32 busNumber, UInt32 numberFrames, AudioBufferList * data)
	{
		OSStatus status = noErr;
		
		AudioBuffer * audioBuffer = data->mBuffers;
		
		// If a converter is present, then we have to resample audio.
		if (converter != nullptr) {
			UInt32 convNumberPackets = audioBuffer->mDataByteSize / deviceIOFormat.mBytesPerPacket;
			
			// Fill sample buffer and convert samples.
			OSStatus convStatus = AudioConverterFillComplexBuffer(converter, fillConverterBuffer, this, &convNumberPackets, data, 0);
			
			if (convStatus != noErr) {
				THROW_IF_FAILED(convStatus, "CoreAudio: Resample audio failed.");
			}
		}
		else {
			UInt32 size = audioBuffer->mDataByteSize;
			
			// Read samples from the source.
			int read = readAudio(size);
			if (read < 1) {
				// EOS
			}
			else {
				std::memcpy(audioBuffer->mData, AudioStream::ioBuffer.data(), read);
				
				audioBuffer->mDataByteSize = read;
			}
		}
		
		return status;
	}
	
	OSStatus CoreAudioInputStream::fillConverterBuffer(AudioConverterRef audioConverter, UInt32 * numberDataPackets, AudioBufferList * data, AudioStreamPacketDescription ** outDataPacketDescription, void * context)
	{
		CoreAudioInputStream * stream = static_cast<CoreAudioInputStream *>(context);

		// Read samples from the source.
		int read = stream->readAudio(*numberDataPackets * stream->deviceIOFormat.mBytesPerPacket);
		
		if (read < 1) {
			// EOS
		}
		else {
			data->mBuffers[0].mData = stream->AudioStream::ioBuffer.data();
		}
		
		// Tell the Audio Converter how much data is in each buffer.
		data->mBuffers[0].mDataByteSize = (read > 0) ? read : 0;
		
		return noErr;
	}
	
}