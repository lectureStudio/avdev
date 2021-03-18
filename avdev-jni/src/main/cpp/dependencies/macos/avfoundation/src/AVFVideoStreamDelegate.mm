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

#import "AVFVideoStreamDelegate.h"
#import "AVFTypeConverter.h"
#import "PixelFormatConverter.h"
#import "Log.h"

@implementation AVFVideoStreamDelegate

- (id) init
{
	self = [super init];
	
	if (self) {
		session = NULL;
		videoOutput = NULL;
		captureCallback = NULL;
		
		converter = NULL;
	}
	
	return self;
}

- (void) dealloc
{
	videoOutput = NULL;
	session = NULL;
	captureCallback = NULL;
	
	[super dealloc];
}

- (void) open: (AVCaptureDevice *) device captureCallback: (avdev::AVFVideoFrameCallback *) callback captureFormat: (avdev::PictureFormat) format frameRate: (float) frameRate
{
	NSError * error;
	
	captureCallback = callback;
	
	// Create and configure session.
	session = [[AVCaptureSession alloc] init];
	
	// Lock the session to begin configuring.
	[session beginConfiguration];
	
    // Select one of supported formats.
    NSArray * devFormats = [device formats];
    AVCaptureDeviceFormat * bestFormat = nil;
    AVFrameRateRange * bestFrameRateRange = nil;
    
    for (AVCaptureDeviceFormat * devFormat in devFormats) {
        const CMVideoDimensions dimension = CMVideoFormatDescriptionGetDimensions(devFormat.formatDescription);
        
        if (dimension.width == format.getWidth() && dimension.height == format.getHeight()) {
            // Select the best match for the frame rate range.
            for (AVFrameRateRange * range in devFormat.videoSupportedFrameRateRanges) {
				float dist = std::abs(bestFrameRateRange.maxFrameRate - frameRate);

                if (dist > std::abs(range.maxFrameRate - frameRate)) {
                    bestFormat = devFormat;
                    bestFrameRateRange = range;
                }
            }
        }
    }

    if ([device lockForConfiguration: &error] == YES && bestFormat != nil) {
        device.activeFormat = bestFormat;
        device.activeVideoMinFrameDuration = bestFrameRateRange.maxFrameDuration;
        device.activeVideoMaxFrameDuration = bestFrameRateRange.maxFrameDuration;
        
        [device unlockForConfiguration];
    }
    else {
		[NSException raise: @"StreamException" format: @"Lock device for configuration failed: %s", error.localizedDescription.UTF8String];
    }
	
	// Create and connect device input.
	AVCaptureDeviceInput * videoInput = [AVCaptureDeviceInput deviceInputWithDevice: device error: &error];
	
	if (!videoInput) {
		[NSException raise: @"StreamException" format: @"Create video input failed: %s", error.localizedDescription.UTF8String];
	}

	if ([session canAddInput: videoInput]) {
		[session addInput: videoInput];
	}
	else {
		[NSException raise: @"StreamException" format: @"Add video input to session failed."];
	}
	
	// Create and connect data output.
	videoOutput = [AVCaptureVideoDataOutput new];
	
	NSNumber * formatType = [NSNumber numberWithInt: avdev::AVFTypeConverter::toApiType(format.getPixelFormat())];
	NSArray * pixelFormats = [videoOutput availableVideoCVPixelFormatTypes];
	
	// If no match found, pick the most efficient output format.
	if (![pixelFormats containsObject: formatType]) {
		formatType = [pixelFormats objectAtIndex: 0];
	}
	
	NSDictionary * settings = @{ (NSString *) kCVPixelBufferPixelFormatTypeKey: formatType,
								 (NSString *) kCVPixelBufferWidthKey: @(format.getWidth()),
								 (NSString *) kCVPixelBufferHeightKey: @(format.getHeight()),
								 AVVideoScalingModeKey: AVVideoScalingModeResizeAspect };
	
	videoOutput.videoSettings = settings;
	videoOutput.alwaysDiscardsLateVideoFrames = YES;

	dispatch_queue_t outputQueue = dispatch_queue_create("AVdevQueue", DISPATCH_QUEUE_SERIAL);
	[videoOutput setSampleBufferDelegate: self queue: outputQueue];
	dispatch_release(outputQueue);
	
	if ([session canAddOutput: videoOutput]) {
		[session addOutput: videoOutput];
	}
	else {
		[NSException raise: @"StreamException" format: @"Add video output to session failed."];
	}
	
	AVCaptureConnection * conn = [videoOutput connectionWithMediaType:AVMediaTypeVideo];

	if (conn.isVideoMinFrameDurationSupported)
		conn.videoMinFrameDuration = bestFrameRateRange.maxFrameDuration;
	if (conn.isVideoMaxFrameDurationSupported)
		conn.videoMaxFrameDuration = bestFrameRateRange.maxFrameDuration;
	
	// Unlock session.
	[session commitConfiguration];
	
	avdev::PixelFormat pixFormat = avdev::AVFTypeConverter::toPixelFormat([formatType intValue]);
	avdev::PictureFormat outputFormat = avdev::PictureFormat(format.getWidth(), format.getHeight(), pixFormat);
	
	if (format != outputFormat) {
		LOGDEV_DEBUG("Format: Device [%s] <> User [%s]", outputFormat.toString().c_str(), format.toString().c_str());
		
		unsigned size = format.getWidth() * format.getHeight() * format.getBytesPerPixel();
		
		[self initFrameBuffer: size];
		
		converter = std::make_shared<avdev::PixelFormatConverter>();
		converter->init(outputFormat, format);
	}
}

- (void) start
{
	// Connect the notifications.
	NSNotificationCenter * nc = [NSNotificationCenter defaultCenter];
	
	[nc addObserver: self
		   selector: @selector(onStreamError:)
			   name: AVCaptureSessionRuntimeErrorNotification
			 object: session];
	
	if (session) {
		[session startRunning];
	}
}

- (void) stop
{
	if (session && [session isRunning]) {
		[session stopRunning];
	}
	
	[[NSNotificationCenter defaultCenter] removeObserver: self];
}

- (void) close
{
	if (videoOutput) {
		[videoOutput release];
	}
	if (session) {
		[session release];
	}
}

- (const avdev::PictureFormat) getPictureFormat
{
	NSDictionary * settings = videoOutput.videoSettings;
	NSNumber * formatType = [settings objectForKey: (id) kCVPixelBufferPixelFormatTypeKey];
	
	unsigned width = [[settings objectForKey: (id) kCVPixelBufferWidthKey] unsignedIntValue];
	unsigned height = [[settings objectForKey: (id) kCVPixelBufferHeightKey] unsignedIntValue];
	avdev::PixelFormat pixFormat = avdev::AVFTypeConverter::toPixelFormat([formatType unsignedIntValue]);
	
	return avdev::PictureFormat(width, height, pixFormat);
}

- (void) initFrameBuffer : (unsigned) length
{
	if (buffer.size() != length) {
		buffer.resize(length);
	}
}

- (void) onStreamError : (NSNotification *) notification
{
	NSError * error = notification.userInfo[AVCaptureSessionErrorKey];

	LOGDEV_ERROR("AVFoundation Error: %s", error.localizedDescription.UTF8String);
}

- (void) captureOutput : (AVCaptureOutput *) captureOutput didOutputSampleBuffer: (CMSampleBufferRef) sampleBuffer fromConnection: (AVCaptureConnection *) connection
{
	// Check if this is the output we are expecting.
	if (captureOutput == videoOutput) {
		CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
		CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
		
		uint8_t * baseAddress = reinterpret_cast<uint8_t *>(CVPixelBufferGetBaseAddress(imageBuffer));
		size_t width = CVPixelBufferGetWidth(imageBuffer);
		size_t height = CVPixelBufferGetHeight(imageBuffer);
		size_t stride = CVPixelBufferGetBytesPerRow(imageBuffer);
		size_t frameSize = stride * height;

		if (converter) {
			converter->convert(baseAddress, buffer.data(), static_cast<int>(width * height));
			
			frameSize = width * height * converter->getOutputFormat().getBytesPerPixel();
			baseAddress = buffer.data();
		}
		
		captureCallback->onVideoFrame(baseAddress, frameSize);
		
		CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
	}
}

@end
