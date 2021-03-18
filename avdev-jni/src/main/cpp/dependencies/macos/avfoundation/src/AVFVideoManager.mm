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

#include "AVFVideoManager.h"
#include "AVFVideoCaptureDevice.h"
#include "Log.h"

namespace avdev
{
	AVFVideoManager::AVFVideoManager() :
		VideoManager()
	{
		// Set up hot-plug listeners.
		NSNotificationCenter * notificationCenter = [NSNotificationCenter defaultCenter];
		
		void (^deviceConnectedBlock)(NSNotification *) = ^(NSNotification * notification) {
			AVCaptureDevice * device = [notification object];
			onDeviceConnected(device);
		};
		
		void (^deviceDisconnectedBlock)(NSNotification *) = ^(NSNotification * notification) {
			AVCaptureDevice * device = [notification object];
			onDeviceDisconnected(device);
		};

		devConnectObserver = [notificationCenter addObserverForName: AVCaptureDeviceWasConnectedNotification
													object: nil
													queue: [NSOperationQueue mainQueue]
													usingBlock: deviceConnectedBlock];
		
		devDisconnectObserver = [notificationCenter addObserverForName: AVCaptureDeviceWasDisconnectedNotification
													object: nil
													queue: [NSOperationQueue mainQueue]
													usingBlock: deviceDisconnectedBlock];
	}

	AVFVideoManager::~AVFVideoManager()
	{
		NSNotificationCenter * notificationCenter = [NSNotificationCenter defaultCenter];
		[notificationCenter removeObserver: devConnectObserver];
		[notificationCenter removeObserver: devDisconnectObserver];
	}
	
	std::set<PVideoCaptureDevice> AVFVideoManager::getVideoCaptureDevices()
	{
		if (captureDevices.empty()) {
			NSArray * devices = [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo];
			
			for (AVCaptureDevice * device in devices) {
				insertDevice(device, false);
			}
		}

		return captureDevices.devices();
	}
	
	void AVFVideoManager::insertDevice(AVCaptureDevice * device, bool notify)
	{
		std::string name([[device localizedName] cStringUsingEncoding: NSUTF8StringEncoding]);
		std::string desc([[device uniqueID] cStringUsingEncoding: NSUTF8StringEncoding]);

		auto captureDevice = std::make_shared<AVFVideoCaptureDevice>(name, desc);
		
		captureDevices.insertDevice(captureDevice);
		
		// Update default device.
		AVCaptureDevice * defaultDevice = [AVCaptureDevice defaultDeviceWithMediaType: AVMediaTypeVideo];
		NSString * defaultID = [defaultDevice uniqueID];
		
		if ([defaultID isEqualToString: [device uniqueID]]) {
			setDefaultCaptureDevice(captureDevice);
		}
		
		if (notify) {
			notifyDeviceConnected(captureDevice);
		}
	}
	
	void AVFVideoManager::removeDevice(AVCaptureDevice * device, bool notify)
	{
		std::string desc([[device uniqueID] cStringUsingEncoding: NSUTF8StringEncoding]);
		
		auto predicate = [desc](const PVideoCaptureDevice & dev) { return desc == dev->getDescriptor(); };
		PVideoCaptureDevice removed = captureDevices.removeDevice(predicate);
		
		if (removed == nullptr) {
			std::string name([[device localizedName] cStringUsingEncoding: NSUTF8StringEncoding]);
			
			LOGDEV_WARN("AVFoundation: Remove device [%s] failed.", name.c_str());
			return;
		}
		
		// Update default device.
		AVCaptureDevice * defaultDevice = [AVCaptureDevice defaultDeviceWithMediaType: AVMediaTypeVideo];
		PVideoCaptureDevice defaultCapture = nullptr;
		
		if (defaultDevice) {
			std::string desc([[defaultDevice uniqueID] cStringUsingEncoding: NSUTF8StringEncoding]);
			
			auto predicate = [desc](const PVideoCaptureDevice & dev) { return desc == dev->getDescriptor(); };
			defaultCapture = captureDevices.findDevice(predicate);
		}
		
		setDefaultCaptureDevice(defaultCapture);
		
		if (notify) {
			notifyDeviceDisconnected(removed);
		}
	}
	
	void AVFVideoManager::onDeviceConnected(AVCaptureDevice * device)
	{
		if ([device hasMediaType: AVMediaTypeVideo]) {
			insertDevice(device, true);
		}
	}
	
	void AVFVideoManager::onDeviceDisconnected(AVCaptureDevice * device)
	{
		if ([device hasMediaType: AVMediaTypeVideo]) {
			removeDevice(device, true);
		}
	}

}