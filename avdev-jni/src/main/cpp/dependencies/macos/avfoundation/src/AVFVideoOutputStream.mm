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

#include "AVFVideoOutputStream.h"
#include "AVdevException.h"
#include "Log.h"

namespace avdev
{
	AVFVideoOutputStream::AVFVideoOutputStream(std::string devDescriptor, PVideoSink sink) :
		VideoOutputStream(sink),
		devDescriptor(devDescriptor),
		streamDelegate(nullptr)
	{
	}

	AVFVideoOutputStream::~AVFVideoOutputStream()
	{
		switch (getState()) {
			case StreamState::STARTED:
				stop();
				close();
				break;
				
			case StreamState::STOPPED:
			case StreamState::OPENED:
				close();
				break;
				
			default:
				break;
		}
	}

	PictureFormat const& AVFVideoOutputStream::getPictureFormat() const
	{
		if (streamDelegate) {
			return [streamDelegate getPictureFormat];
		}

		return VideoOutputStream::getPictureFormat();
	}

	float AVFVideoOutputStream::getFrameRate() const
	{
		// Get the device by ID.
		NSString * deviceID = [NSString stringWithUTF8String:devDescriptor.c_str()];
		AVCaptureDevice * device = [AVCaptureDevice deviceWithUniqueID: deviceID];

		float frameRate = 1 / CMTimeGetSeconds(device.activeVideoMinFrameDuration);
		
		return frameRate;
	}

	void AVFVideoOutputStream::openInternal()
	{
		PictureFormat format = VideoOutputStream::getPictureFormat();
		float frameRate = VideoOutputStream::getFrameRate();

		// Get the device by ID.
		NSString * deviceID = [NSString stringWithUTF8String:devDescriptor.c_str()];
		AVCaptureDevice * device = [AVCaptureDevice deviceWithUniqueID: deviceID];
		
		// Call the stream delegate.
		streamDelegate = [[AVFVideoStreamDelegate alloc] init];

		@try {
			[streamDelegate open : device captureCallback: this captureFormat: format frameRate: frameRate];
		}
		@catch (NSException * exception) {
			std::string message([exception.reason UTF8String]);
			
			throw AVdevException("AVFoundation: Open stream failed [%s]", message.c_str());
		}
	}

	void AVFVideoOutputStream::closeInternal()
	{
		if (streamDelegate) {
			@try {
				[streamDelegate close];
				[streamDelegate release];
			}
			@catch (NSException * exception) {
				std::string message([exception.reason UTF8String]);
				
				throw AVdevException("AVFoundation: Close stream failed [%s]", message.c_str());
			}
		}
		
		streamDelegate = nullptr;
	}

	void AVFVideoOutputStream::startInternal()
	{
		if (streamDelegate) {
			@try {
				[streamDelegate start];
			}
			@catch (NSException * exception) {
				std::string message([exception.reason UTF8String]);
				
				throw AVdevException("AVFoundation: Start stream failed [%s]", message.c_str());
			}
		}
	}

	void AVFVideoOutputStream::stopInternal()
	{
		if (streamDelegate) {
			@try {
				[streamDelegate stop];
			}
			@catch (NSException * exception) {
				std::string message([exception.reason UTF8String]);
				
				throw AVdevException("AVFoundation: Stop stream failed [%s]", message.c_str());
			}
		}
	}

	void AVFVideoOutputStream::onVideoFrame(const uint8_t * frameBuffer, size_t frameSize)
	{
		writeVideoFrame(frameBuffer, frameSize);
	}

}
