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

#ifndef AVDEV_AVF_VIDEO_STREAM_DELEGATE_H_
#define AVDEV_AVF_VIDEO_STREAM_DELEGATE_H_

#import <AVFoundation/AVFoundation.h>
#import <cstdint>
#import <vector>

#import "AVFVideoFrameCallback.h"
#import "PixelFormatConverter.h"
#import "PictureFormat.h"

@interface AVFVideoStreamDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
{
	@private
		AVCaptureSession * session;
		AVCaptureVideoDataOutput * videoOutput;
	
		avdev::AVFVideoFrameCallback * captureCallback;
	
		std::shared_ptr<avdev::PixelFormatConverter> converter;
	
		std::vector<std::uint8_t> buffer;
}

- (id) init;
- (void) dealloc;
- (void) open : (AVCaptureDevice *) device captureCallback: (avdev::AVFVideoFrameCallback *) callback captureFormat: (avdev::PictureFormat) format frameRate: (float) frameRate;
- (void) start;
- (void) stop;
- (void) close;
- (const avdev::PictureFormat) getPictureFormat;
- (void) initFrameBuffer : (unsigned) length;

@end

#endif