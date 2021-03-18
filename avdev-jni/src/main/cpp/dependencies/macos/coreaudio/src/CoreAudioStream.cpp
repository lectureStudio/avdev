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

#include "CoreAudioStream.h"
#include "CoreAudioTypeConverter.h"
#include "AVdevException.h"
#include "Log.h"

namespace avdev
{	
	CoreAudioStream::CoreAudioStream(AudioDeviceID devID, AudioObjectPropertyScope scope) :
		devID(devID),
		scope(scope),
		auHAL(nullptr),
		converter(nullptr),
		devBuffer(),
		convBuffer(),
		state(StreamState::CLOSED),
		format(AudioFormat(SampleFormat::S16LE, 44100, 1))
	{
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioObjectPropertySelectorWildcard;
		pa.mScope = kAudioObjectPropertyScopeWildcard;
		pa.mElement = kAudioObjectPropertyElementWildcard;
		
		OSStatus status = AudioObjectAddPropertyListener(devID, &pa, deviceListenerProc, this);
		THROW_IF_FAILED(status, "CoreAudio: Add device listener failed.");
	}
	
	CoreAudioStream::~CoreAudioStream()
	{
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioObjectPropertySelectorWildcard;
		pa.mScope = kAudioObjectPropertyScopeWildcard;
		pa.mElement = kAudioObjectPropertyElementWildcard;
		
		AudioObjectRemovePropertyListener(devID, &pa, deviceListenerProc, this);
	}
	
	float CoreAudioStream::getVolume()
	{
		if (state == StreamState::CLOSED) {
			throw AVdevException("CoreAudio: Get master volume failed. Stream is closed.");
		}
		
		float volume = 0;
		UInt32 dataSize = sizeof(volume);
		
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioDevicePropertyVolumeScalar;
		pa.mScope = scope;
		pa.mElement = kAudioObjectPropertyElementMaster;
		
		if (AudioObjectHasProperty(devID, &pa)) {
			OSStatus status = AudioObjectGetPropertyData(devID, &pa, 0, nullptr, &dataSize, &volume);
			THROW_IF_FAILED(status, "CoreAudio: Get master volume failed.");
		}
		else {
			// Get the average volume of all channels.
			for (int i = 1; i <= deviceFormat.mChannelsPerFrame; i++) {
				pa.mElement = i;
				
				if (AudioObjectHasProperty(devID, &pa)) {
					Float32 channelVolume = 0;
					UInt32 size = sizeof(channelVolume);
					OSStatus status = AudioObjectGetPropertyData(devID, &pa, 0, nullptr, &size, &channelVolume);
					THROW_IF_FAILED(status, "CoreAudio: Get volume of channel %d failed.", i);
					
					volume += channelVolume;
				}
			}
			
			volume /= deviceFormat.mChannelsPerFrame;
		}

		return volume;
	}
	
	void CoreAudioStream::setVolume(float volume)
	{
		if (state == StreamState::CLOSED) {
			throw AVdevException("CoreAudio: Set master volume failed. Stream is closed.");
		}
		
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioDevicePropertyVolumeScalar;
		pa.mScope = scope;
		pa.mElement = kAudioObjectPropertyElementMaster;
		
		if (canSetChannelVolume(kAudioObjectPropertyElementMaster)) {
			OSStatus status = AudioObjectSetPropertyData(devID, &pa, 0, nullptr, sizeof(volume), &volume);
			THROW_IF_FAILED(status, "CoreAudio: Set master volume failed.");
		}
		else {
			// Set volume for each channel.
			for (int i = 1; i <= deviceFormat.mChannelsPerFrame; i++) {
				pa.mElement = i;

				if (canSetChannelVolume(i)) {
					OSStatus status = AudioObjectSetPropertyData(devID, &pa, 0, NULL, sizeof(volume), &volume);
					THROW_IF_FAILED(status, "CoreAudio: Set volume for channel %d failed.", i);
				}
			}
		}
	}
	
	bool CoreAudioStream::getMute()
	{
		if (state == StreamState::CLOSED) {
			throw AVdevException("CoreAudio: Get stream mute state failed. Stream is closed.");
		}
		
		UInt32 mute = 0;
		UInt32 dataSize = sizeof(UInt32);
		
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioDevicePropertyMute;
		pa.mScope = scope;
		pa.mElement = kAudioObjectPropertyElementMaster;
		
		OSStatus status = AudioObjectGetPropertyData(devID, &pa, 0, nullptr, &dataSize, &mute);
		THROW_IF_FAILED(status, "CoreAudio: Get stream mute state failed.");
		
		return (mute != 0);
	}
	
	void CoreAudioStream::setMute(bool mute)
	{
		if (state == StreamState::CLOSED) {
			throw AVdevException("CoreAudio: Set stream mute state failed. Stream is closed.");
		}
		
		UInt32 muteInt = mute;
		
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioDevicePropertyMute;
		pa.mScope = scope;
		pa.mElement = kAudioObjectPropertyElementMaster;
		
		OSStatus status = AudioObjectSetPropertyData(devID, &pa, 0, nullptr, sizeof(muteInt), &muteInt);
		THROW_IF_FAILED(status, "CoreAudio: Set stream mute state failed.");
	}
	
	float CoreAudioStream::getBufferLatency()
	{
		if (state == StreamState::CLOSED) {
			throw AVdevException("CoreAudio: Get IO buffer frame size failed. Stream is closed.");
		}
		
		UInt32 bufferFrameSize;
		
		OSStatus status = getIOBufferFrameSize(auHAL, &bufferFrameSize);
		THROW_IF_FAILED(status, "CoreAudio: Get IO buffer frame size failed.");
		
		return static_cast<float>(bufferFrameSize / deviceFormat.mSampleRate * 1000);
	}
	
	void CoreAudioStream::open(CoreaAudioType type, AudioFormat format, unsigned latency)
	{
		AudioUnitScope inScope;
		AudioUnitScope outScope;
		AudioUnitElement inBusType;
		AudioUnitElement outBusType;
		AudioUnitPropertyID callbackType;
		
		switch (type) {
			case CoreaAudioType::CoreAudioInput:
				inScope = kAudioUnitScope_Output;
				outScope = kAudioUnitScope_Input;
				inBusType = 0;	// output bus
				outBusType = 1;	// input bus
				callbackType = kAudioUnitProperty_SetRenderCallback;
				break;
			case CoreaAudioType::CoreAudioOutput:
				inScope = kAudioUnitScope_Input;
				outScope = kAudioUnitScope_Output;
				inBusType = 1;	// input bus
				outBusType = 0;	// output bus
				callbackType = kAudioOutputUnitProperty_SetInputCallback;
				break;
			default:
				break;
		}
		
		AudioComponentDescription desc;
		desc.componentType = kAudioUnitType_Output;
		desc.componentSubType = kAudioUnitSubType_HALOutput;
		desc.componentManufacturer = kAudioUnitManufacturer_Apple;
		desc.componentFlags = 0;
		desc.componentFlagsMask = 0;
		
		AudioComponent comp = AudioComponentFindNext(nullptr, &desc);
		THROW_IF_FAILED(comp == nullptr, "CoreAudio: Find audio component failed.");
		
		OSStatus status = AudioComponentInstanceNew(comp, &auHAL);
		THROW_IF_FAILED(status, "CoreAudio: Create instance of audio component failed.");
		
		// Enable input and disabling output for AudioOutputUnit.
		UInt32 enableIO = 1;
		
		status = AudioUnitSetProperty(auHAL, kAudioOutputUnitProperty_EnableIO, inScope, inBusType, &enableIO, sizeof(enableIO));
		THROW_IF_FAILED(status, "CoreAudio: Enable AUHAL input failed.");
		
		enableIO = 0;
		
		status = AudioUnitSetProperty(auHAL, kAudioOutputUnitProperty_EnableIO, outScope, outBusType, &enableIO, sizeof(enableIO));
		THROW_IF_FAILED(status, "CoreAudio: Disable AUHAL output failed.");
	
		// Select the desired device.
		status = AudioUnitSetProperty(auHAL, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &devID, sizeof(devID));
		THROW_IF_FAILED(status, "CoreAudio: Set current AUHAL device failed.");
		
		UInt32 dataSize = sizeof(AudioStreamBasicDescription);
		
		// Get the device format.
		status = AudioUnitGetProperty(auHAL, kAudioUnitProperty_StreamFormat, inScope, inBusType, &deviceFormat, &dataSize);
		THROW_IF_FAILED(status, "CoreAudio: Get AUHAL stream format failed.");
		
		// Set up the desired format.
		AudioStreamBasicDescription desiredFormat = CoreAudioTypeConverter::toStreamDescription(format);
		
		// Sample rate must match the sample rate of the device.
		desiredFormat.mSampleRate = deviceFormat.mSampleRate;
		
		status = AudioUnitSetProperty(auHAL, kAudioUnitProperty_StreamFormat, outScope, inBusType, &desiredFormat, sizeof(desiredFormat));
		THROW_IF_FAILED(status, "CoreAudio: Set AUHAL stream format failed.");

		// Set the buffer frame size.
		UInt32 minimum;
		UInt32 maximum;
		
		status = getIOBufferFrameSizeRange(auHAL, &minimum, &maximum);
		THROW_IF_FAILED(status, "CoreAudio: Get IO buffer frame size range failed.");
		
		float minMs = static_cast<float>(minimum / deviceFormat.mSampleRate * 1000);
		float maxMs = static_cast<float>(maximum / deviceFormat.mSampleRate * 1000);
		
		LOGDEV_DEBUG("CoreAudio: Buffer frame size range [%d to %d] - [%f to %f] ms.", minimum, maximum, minMs, maxMs);
		
		UInt32 bufferFrameSizeUser = (latency / 1000.f) * deviceFormat.mSampleRate + 0.5f;
		UInt32 bufferFrameSize;
	
		status = getIOBufferFrameSize(auHAL, &bufferFrameSize);
		THROW_IF_FAILED(status, "CoreAudio: Get IO buffer frame size failed.");
		
		LOGDEV_DEBUG("CoreAudio: Buffer frame size [Device: %d] [User: %d].", bufferFrameSize, bufferFrameSizeUser);
		
		// Only set the buffer frame size if the desired buffer size is lower than the current buffer size.
		// Avoid to override other applications low-latency constraints.
		if (bufferFrameSizeUser < bufferFrameSize) {
			status = setIOBufferFrameSize(auHAL, bufferFrameSizeUser);
			THROW_IF_FAILED(status, "CoreAudio: Set IO buffer frame size failed.");
		}
		
		// Register input procedure to receive data from the input device.
		AURenderCallbackStruct callback;
		callback.inputProc = &ioProc;
		callback.inputProcRefCon = this;
		
		status = AudioUnitSetProperty(auHAL, callbackType, kAudioUnitScope_Global, 0, &callback, sizeof(callback));
		THROW_IF_FAILED(status, "CoreAudio: Set AUHAL input callback failed.");
		
		status = AudioUnitInitialize(auHAL);
		THROW_IF_FAILED(status, "CoreAudio: Initialize AUHAL unit failed.");
		
		// Get AUHAL output format.
		dataSize = sizeof(deviceIOFormat);

		status = AudioUnitGetProperty(auHAL, kAudioUnitProperty_StreamFormat, outScope, inBusType, &deviceIOFormat, &dataSize);
		THROW_IF_FAILED(status, "CoreAudio: Get AUHAL stream IO format failed.");
		
		LOGDEV_DEBUG("CoreAudio: Device Format [Sample Rate: %d] [Channels: %d] [Bytes Per Frame: %d].",
					 (int)deviceFormat.mSampleRate,
					 deviceFormat.mChannelsPerFrame,
					 deviceFormat.mBytesPerFrame);
		LOGDEV_DEBUG("CoreAudio: IO Format [Sample Rate: %d] [Channels: %d] [Bytes Per Frame: %d].",
					 (int)deviceIOFormat.mSampleRate,
					 deviceIOFormat.mChannelsPerFrame,
					 deviceIOFormat.mBytesPerFrame);
		LOGDEV_DEBUG("CoreAudio: User Format [Sample Rate: %d] [Channels: %d] [Bytes Per Frame: %d].",
					 format.getSampleRate(),
					 format.getChannels(),
					 format.bitsPerSample() / 8);
		
		// Get the number of frames in the IO buffer(s).
		status = getIOBufferFrameSize(auHAL, &bufferFrameSize);
		THROW_IF_FAILED(status, "CoreAudio: Get buffer frame size of AUHAL unit failed.");
		
		// Init audio sample buffer.
		initIOBuffer(bufferFrameSize * deviceIOFormat.mBytesPerFrame);

		// Create resampler, if formats do not match.
		AudioFormat devAudioFormat = CoreAudioTypeConverter::toAudioFormat(deviceIOFormat);
		
		if (devAudioFormat != format) {
			initResampler(type, format, bufferFrameSize);
		}
		
		bufferFrameSizeUser = (latency / 1000.f) * format.getSampleRate() * format.getChannels() * desiredFormat.mBytesPerFrame;
		
		LOGDEV_DEBUG("CoreAudio: Buffer latency [%ld Bytes] - [%d ms].", bufferFrameSizeUser, latency);
		
		initDeviceBuffer(bufferFrameSizeUser);
		
		this->type = type;
		this->format = format;
		this->latency = latency;
		this->state = StreamState::OPENED;
	}
	
	void CoreAudioStream::start()
	{
		OSStatus status = AudioOutputUnitStart(auHAL);
		THROW_IF_FAILED(status, "CoreAudio: Start AUHAL unit failed.");
		
		state = StreamState::STARTED;
	}
	
	void CoreAudioStream::stop()
	{
		OSStatus status = AudioOutputUnitStop(auHAL);
		THROW_IF_FAILED(status, "CoreAudio: Stop AUHAL unit failed.");
		
		state = StreamState::STOPPED;
	}
	
	void CoreAudioStream::close()
	{
		OSStatus status = AudioUnitUninitialize(auHAL);
		THROW_IF_FAILED(status, "CoreAudio: Uninitialize AUHAL unit failed.");
		
		status = AudioComponentInstanceDispose(auHAL);
		THROW_IF_FAILED(status, "CoreAudio: Dispose AUHAL unit failed.");
		
		if (converter != nullptr) {
			status = AudioConverterDispose(converter);
			THROW_IF_FAILED(status, "CoreAudio: Dispose audio converter failed.");
		}
		
		state = StreamState::CLOSED;
	}

	bool CoreAudioStream::canSetChannelVolume(unsigned channel) {
		Boolean settable = false;
		
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioDevicePropertyVolumeScalar;
		pa.mScope = scope;
		pa.mElement = channel;
		
		OSStatus status = AudioObjectIsPropertySettable(devID, &pa, &settable);
		
		return (status == noErr) ? settable : false;
	}
	
	OSStatus CoreAudioStream::getIOBufferFrameSizeRange(AudioUnit auHAL, UInt32 * outMinimum, UInt32 * outMaximum) {
		AudioValueRange range = { 0, 0 };
		UInt32 dataSize = sizeof(AudioValueRange);
		
		OSStatus status = AudioUnitGetProperty(auHAL, kAudioDevicePropertyBufferFrameSizeRange, kAudioUnitScope_Global, 0, &range, &dataSize);
		
		if (status == 0) {
			*outMinimum = range.mMinimum;
			*outMaximum = range.mMaximum;
		}
		
		return status;
	}
	
	OSStatus CoreAudioStream::getIOBufferFrameSize(AudioUnit auHAL, UInt32 * outIOBufferFrameSize) {
		UInt32 dataSize = sizeof(UInt32);
		
		return AudioUnitGetProperty(auHAL, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, outIOBufferFrameSize, &dataSize);
	}
	
	OSStatus CoreAudioStream::setIOBufferFrameSize(AudioUnit auHAL, UInt32 ioBufferFrameSize) {
		return AudioUnitSetProperty(auHAL, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &ioBufferFrameSize, sizeof(UInt32));
	}
	
	void CoreAudioStream::initIOBuffer(unsigned int bufferSize)
	{
		if (devBuffer.size() != bufferSize) {
			devBuffer.resize(bufferSize);
		}
		
		bufferList.mNumberBuffers = 1;
		
		AudioBuffer * audioBuffer = bufferList.mBuffers;
		audioBuffer->mNumberChannels = deviceIOFormat.mChannelsPerFrame;
		audioBuffer->mDataByteSize = bufferSize;
		audioBuffer->mData = devBuffer.data();
		
		initDeviceBuffer(bufferSize);
	}
	
	void CoreAudioStream::initResampler(CoreaAudioType type, AudioFormat format, UInt32 bufferFrameSize) {
		OSStatus status;
		
		converterFormat = CoreAudioTypeConverter::toStreamDescription(format);

		if (type == CoreaAudioType::CoreAudioInput) {
			status = AudioConverterNew(&converterFormat, &deviceIOFormat, &converter);
		}
		else {
			status = AudioConverterNew(&deviceIOFormat, &converterFormat, &converter);
		}

		THROW_IF_FAILED(status, "CoreAudio: Create audio converter failed.");
		
		UInt32 quality = kAudioConverterQuality_Max;
		UInt32 complexity = kAudioConverterSampleRateConverterComplexity_Normal;

		status = AudioConverterSetProperty(converter, kAudioConverterSampleRateConverterQuality, sizeof(quality), &quality);
		THROW_IF_FAILED(status, "CoreAudio: Set audio converter quality failed.");
		
		status = AudioConverterSetProperty(converter, kAudioConverterSampleRateConverterComplexity, sizeof(complexity), &complexity);
		THROW_IF_FAILED(status, "CoreAudio: Set audio converter complexity failed.");
		
		// Initialise converter buffer.
		float convSamples = static_cast<float>(bufferFrameSize / deviceFormat.mSampleRate * converterFormat.mSampleRate);
		UInt32 bufferSize = static_cast<UInt32>(convSamples * converterFormat.mBytesPerFrame + 0.5f);
		
		if (convBuffer.size() != bufferSize) {
			convBuffer.resize(bufferSize);
		}
		
		convBufferList.mNumberBuffers = 1;
		
		AudioBuffer * convAudioBuffer = convBufferList.mBuffers;
		convAudioBuffer->mNumberChannels = converterFormat.mChannelsPerFrame;
		convAudioBuffer->mDataByteSize = bufferSize;
		convAudioBuffer->mData = convBuffer.data();
	}
	
	void CoreAudioStream::restart()
	{
		if (state != StreamState::CLOSED) {
			bool started = (state == StreamState::STARTED);
			if (started) {
				stop();
			}
			
			close();
			
			open(type, format, latency);
			
			if (started) {
				start();
			}
		}
	}
	
	OSStatus CoreAudioStream::ioProc(void * context, AudioUnitRenderActionFlags * actionFlags, const AudioTimeStamp * timeStamp, UInt32 busNumber, UInt32 numberFrames, AudioBufferList * data)
	{
		CoreAudioStream * const stream = static_cast<CoreAudioStream *>(context);
		
		// Adapt in case the device buffer size has been changed.
		UInt32 frameBytes = numberFrames * stream->deviceIOFormat.mBytesPerFrame;
		AudioBuffer * audioBuffer = stream->bufferList.mBuffers;
		
		if (frameBytes != audioBuffer->mDataByteSize) {
			LOGDEV_DEBUG("CoreAudio: Buffer size changed [%d to %d].", audioBuffer->mDataByteSize, frameBytes);
			
			if (frameBytes > audioBuffer->mDataByteSize) {
				// Resize stream buffer.
				stream->initIOBuffer(frameBytes);
			}
			
			audioBuffer->mDataByteSize = frameBytes;
		}
		
		OSStatus status = stream->processAudio(actionFlags, timeStamp, busNumber, numberFrames, data);

		return status;
	}
	
	OSStatus CoreAudioStream::deviceListenerProc(AudioObjectID objectID, UInt32 numberAddresses, const AudioObjectPropertyAddress * addresses, void * clientData)
	{
		CoreAudioStream * const stream = static_cast<CoreAudioStream *>(clientData);
		
		switch (addresses->mSelector) {
			case kAudioDevicePropertyBufferSize:
			case kAudioDevicePropertyBufferSizeRange:
			case kAudioDevicePropertyBufferFrameSize:
			case kAudioDevicePropertyDeviceIsAlive:
			case kAudioDevicePropertyDeviceIsRunning:
			case kAudioDevicePropertyDataSource:
			case kAudioDevicePropertyPlayThru:
				break;
				
			case kAudioDevicePropertyNominalSampleRate:
			case kAudioStreamPropertyPhysicalFormat:
			case kAudioDevicePropertyStreamFormat:
				stream->restart();
				break;
			
			case kAudioDevicePropertyVolumeScalar:
				
				break;
				
			case kAudioDevicePropertyMute:
				
				break;
		}
		
		return noErr;
	}
	
}
