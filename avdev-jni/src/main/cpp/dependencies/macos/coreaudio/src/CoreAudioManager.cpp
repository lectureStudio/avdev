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

#include "CoreAudioManager.h"
#include "CoreAudioPlaybackDevice.h"
#include "CoreAudioCaptureDevice.h"
#include "MacOSHelper.h"
#include "Log.h"

#import <Foundation/Foundation.h>

namespace avdev
{
	CoreAudioManager::CoreAudioManager() :
		AudioManager()
	{
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioObjectPropertySelectorWildcard;
		pa.mScope = kAudioObjectPropertyScopeWildcard;
		pa.mElement = kAudioObjectPropertyElementWildcard;

		OSStatus status = AudioObjectAddPropertyListener(kAudioObjectSystemObject, &pa, deviceListenerProc, this);
		THROW_IF_FAILED(status, "CoreAudio: Add device hotplug listener failed.");
	}
	
	CoreAudioManager::~CoreAudioManager()
	{
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioObjectPropertySelectorWildcard;
		pa.mScope = kAudioObjectPropertyScopeWildcard;
		pa.mElement = kAudioObjectPropertyElementWildcard;
		
		AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &pa, deviceListenerProc, this);
	}

	std::set<PAudioCaptureDevice> CoreAudioManager::getAudioCaptureDevices()
	{
		if (captureDevices.empty()) {
			enumerateDevices(kAudioObjectPropertyScopeInput);
		}
		
		return captureDevices.devices();
	}
	
	std::set<PAudioPlaybackDevice> CoreAudioManager::getAudioPlaybackDevices()
	{
		if (playbackDevices.empty()) {
			enumerateDevices(kAudioObjectPropertyScopeOutput);
		}
		
		return playbackDevices.devices();
	}

	void CoreAudioManager::enumerateDevices(AudioObjectPropertyScope scope) {
		// Get default device ID.
		AudioDeviceID defaultID = getDefaultDeviceID(scope);
		UInt32 dataSize;
		
		// Get all devices.
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioHardwarePropertyDevices;
		pa.mScope = kAudioObjectPropertyScopeGlobal;
		pa.mElement = kAudioObjectPropertyElementMaster;
		
		OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &pa, 0, nullptr, &dataSize);
		THROW_IF_FAILED(status, "CoreAudio: Enumerate audio endpoints failed.");
	
		const int numDevices = dataSize / sizeof(AudioDeviceID);
		AudioDeviceID * devIDs = new AudioDeviceID[numDevices];
		
		status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &pa, 0, nullptr, &dataSize, devIDs);
		THROW_IF_FAILED(status, "CoreAudio: Enumerate audio endpoints failed.");
		
		for (int i = 0; i < numDevices; i++) {
			std::shared_ptr<AudioDevice> device = createAudioDevice(devIDs[i], scope);
			
			if (device != nullptr) {
				insertAudioDevice(device, scope);
				
				// Set default device.
				bool isDefault = (defaultID == devIDs[i]);

				if (isDefault && scope == kAudioObjectPropertyScopeInput) {
					setDefaultCaptureDevice(std::static_pointer_cast<CoreAudioCaptureDevice>(device));
				}
				else if (isDefault && scope == kAudioObjectPropertyScopeOutput) {
					setDefaultPlaybackDevice(std::static_pointer_cast<CoreAudioPlaybackDevice>(device));
				}
			}
		}
		
		delete[] devIDs;
	}
	
	void CoreAudioManager::onDevicesChanged()
	{
		UInt32 dataSize;
		
		// Get all devices.
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioHardwarePropertyDevices;
		pa.mScope = kAudioObjectPropertyScopeGlobal;
		pa.mElement = kAudioObjectPropertyElementMaster;
		
		OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &pa, 0, nullptr, &dataSize);
		THROW_IF_FAILED(status, "CoreAudio: Enumerate audio endpoints failed.");
		
		const int numDevices = dataSize / sizeof(AudioDeviceID);
		AudioDeviceID * devIDs = new AudioDeviceID[numDevices];
		
		status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &pa, 0, nullptr, &dataSize, devIDs);
		THROW_IF_FAILED(status, "CoreAudio: Enumerate audio endpoints failed.");
		
		// Check, if a new device is available.
		for (int i = 0; i < numDevices; i++) {
			bool found = false;
			
			for (const PAudioCaptureDevice & dev : captureDevices.devices()) {
				AudioDeviceID devID = std::stoi(dev->getDescriptor());
				
				if (devIDs[i] == devID) {
					found = true;
					break;
				}
			}
			
			for (const PAudioPlaybackDevice & dev : playbackDevices.devices()) {
				AudioDeviceID devID = std::stoi(dev->getDescriptor());
				
				if (devIDs[i] == devID) {
					found = true;
					break;
				}
			}
			
			if (!found) {
				// New device found.
				unsigned inChannels = getChannelCount(devIDs[i], kAudioObjectPropertyScopeInput);
				unsigned outChannels = getChannelCount(devIDs[i], kAudioObjectPropertyScopeOutput);
				
				if (inChannels > 0) {
					AudioObjectPropertyScope scope = kAudioObjectPropertyScopeInput;
					std::shared_ptr<AudioDevice> device = createAudioDevice(devIDs[i], scope);
					
					if (device != nullptr) {
						insertAudioDevice(device, scope);
						
						// Update default capture device.
						PAudioCaptureDevice def = getDefaultCaptureDevice();
						onDefaultDeviceChanged<PAudioCaptureDevice>(scope, captureDevices, def);
						
						notifyDeviceConnected(device);
					}
				}
				if (outChannels > 0) {
					AudioObjectPropertyScope scope = kAudioObjectPropertyScopeOutput;
					std::shared_ptr<AudioDevice> device = createAudioDevice(devIDs[i], scope);
					
					if (device != nullptr) {
						insertAudioDevice(device, scope);
						
						// Update default playback device.
						PAudioPlaybackDevice def = getDefaultPlaybackDevice();
						onDefaultDeviceChanged<PAudioPlaybackDevice>(scope, playbackDevices, def);
						
						notifyDeviceConnected(device);
					}
				}
			}
		}
		
		// Check, if a device was disconnected.
		checkDeviceGone<PAudioCaptureDevice>(captureDevices, devIDs, numDevices, kAudioObjectPropertyScopeInput);
		checkDeviceGone<PAudioPlaybackDevice>(playbackDevices, devIDs, numDevices, kAudioObjectPropertyScopeOutput);
		
		delete[] devIDs;
	}
	
	template<typename T>
	void CoreAudioManager::onDefaultDeviceChanged(AudioObjectPropertyScope scope, DeviceList<T> & devices, T & device)
	{
		AudioDeviceID defaultID = getDefaultDeviceID(scope);
		AudioDeviceID deviceID = (device == nullptr) ? 0 : std::stoi(device->getDescriptor());
		
		if (defaultID != deviceID) {
			auto predicate = [defaultID](const T & dev) {
				AudioDeviceID devID = std::stoi(dev->getDescriptor());
				
				return defaultID == devID;
			};
			
			T found = devices.findDevice(predicate);
			
			if (found == nullptr) {
				LOGDEV_WARN("CoreAudio: Find device failed.");
				return;
			}
			
			switch (scope) {
				case kAudioObjectPropertyScopeInput:
					setDefaultCaptureDevice(std::dynamic_pointer_cast<CoreAudioCaptureDevice>(found));
					break;
					
				case kAudioObjectPropertyScopeOutput:
					setDefaultPlaybackDevice(std::dynamic_pointer_cast<CoreAudioPlaybackDevice>(found));
					break;
			}
		}
	}
	
	template<typename T>
	void CoreAudioManager::checkDeviceGone(DeviceList<T> & devices, AudioDeviceID * devIDs, const int numDevIDs, AudioObjectPropertyScope scope)
	{
		auto predicate = [devIDs, numDevIDs](const T & dev) {
			AudioDeviceID devID = std::stoi(dev->getDescriptor());
			bool found = false;
			
			for (int i = 0; i < numDevIDs; i++) {
				if (devIDs[i] == devID) {
					found = true;
					break;
				}
			}

			return !found;
		};
		
		T removed = devices.removeDevice(predicate);
		
		if (removed != nullptr) {
			switch (scope) {
				case kAudioObjectPropertyScopeInput:
				{
					PAudioCaptureDevice def = getDefaultAudioCaptureDevice();
					PAudioCaptureDevice rem = std::dynamic_pointer_cast<CoreAudioCaptureDevice>(removed);
					if (rem == def) {
						onDefaultDeviceChanged<PAudioCaptureDevice>(scope, captureDevices, def);
					}
					break;
				}
				
				case kAudioObjectPropertyScopeOutput:
				{
					PAudioPlaybackDevice def = getDefaultAudioPlaybackDevice();
					PAudioPlaybackDevice rem = std::dynamic_pointer_cast<CoreAudioPlaybackDevice>(removed);
					if (rem == def) {
						onDefaultDeviceChanged<PAudioPlaybackDevice>(scope, playbackDevices, def);
					}
					break;
				}
			}

			notifyDeviceDisconnected(removed);
		}
	}

	std::shared_ptr<AudioDevice> CoreAudioManager::createAudioDevice(AudioDeviceID deviceID, AudioObjectPropertyScope scope) {
		std::shared_ptr<AudioDevice> device = nullptr;
		CFStringRef devNameRef;
		UInt32 dataSize = sizeof(devNameRef);
		
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioObjectPropertyName;
		pa.mScope = kAudioObjectPropertyScopeGlobal;
		pa.mElement = kAudioObjectPropertyElementMaster;
		
		OSStatus status = AudioObjectGetPropertyData(deviceID, &pa, 0, nullptr, &dataSize, &devNameRef);
		THROW_IF_FAILED(status, "CoreAudio: Get device name failed.");
		
		CFIndex length = CFStringGetLength(devNameRef) + 1;
		
		char deviceName[length];

		CFStringGetCString(devNameRef, deviceName, length, kCFStringEncodingUTF8);
		CFRelease(devNameRef);

		std::string name(deviceName, length);
		std::string id = std::to_string(deviceID);

		unsigned channels = getChannelCount(deviceID, scope);
		
		if (channels > 0) {
			if (scope == kAudioObjectPropertyScopeOutput) {
				device = std::make_shared<CoreAudioPlaybackDevice>(name, id, deviceID);
			}
			else if (scope == kAudioObjectPropertyScopeInput) {
				device = std::make_shared<CoreAudioCaptureDevice>(name, id, deviceID);
			}
		}
		
		return device;
	}
	
	bool CoreAudioManager::insertAudioDevice(std::shared_ptr<AudioDevice> device, AudioObjectPropertyScope scope)
	{
		if (device == nullptr)
			return false;
		
		if (scope == kAudioObjectPropertyScopeOutput) {
			return playbackDevices.insertDevice(std::static_pointer_cast<CoreAudioPlaybackDevice>(device));
		}
		else if (scope == kAudioObjectPropertyScopeInput) {
			return captureDevices.insertDevice(std::static_pointer_cast<CoreAudioCaptureDevice>(device));
		}
		
		return false;
	}
	
	int CoreAudioManager::getChannelCount(AudioDeviceID deviceID, AudioObjectPropertyScope scope) {
		int channels = 0;
		UInt32 dataSize;
		
		AudioObjectPropertyAddress pa;
		pa.mSelector = kAudioDevicePropertyStreamConfiguration;
		pa.mElement = kAudioObjectPropertyElementMaster;
		pa.mScope = scope;
		
		if (AudioObjectGetPropertyDataSize(deviceID, &pa, 0, nullptr, &dataSize) == noErr) {
			AudioBufferList * buffers = new AudioBufferList[dataSize];

			if (AudioObjectGetPropertyData(deviceID, &pa, 0, nullptr, &dataSize, buffers) == noErr) {
				for (int i = 0; i < buffers->mNumberBuffers; i++) {
					channels += buffers->mBuffers[i].mNumberChannels;
				}
			}
			
			delete[] buffers;
		}
		
		return channels;
	}
	
	AudioDeviceID CoreAudioManager::getDefaultDeviceID(AudioObjectPropertyScope scope) {
		AudioDeviceID defaultID = 0;
		UInt32 dataSize = sizeof(AudioDeviceID);
		
		AudioObjectPropertyAddress pa;
		pa.mScope = kAudioObjectPropertyScopeGlobal;
		pa.mElement = kAudioObjectPropertyElementMaster;
		
		switch (scope) {
			case kAudioObjectPropertyScopeInput:
				pa.mSelector = kAudioHardwarePropertyDefaultInputDevice;
				break;
				
			case kAudioObjectPropertyScopeOutput:
				pa.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
				break;
		}
		
		OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &pa, 0, nullptr, &dataSize, &defaultID);
		
		if (status != noErr) {
			LOGDEV_ERROR("CoreAudio: Get default device ID failed: Status = %d.", status);
		}
		
		return defaultID;
	}
	
	OSStatus CoreAudioManager::deviceListenerProc(AudioObjectID objectID, UInt32 numberAddresses, const AudioObjectPropertyAddress addresses[], void * clientData) {
		CoreAudioManager * const manager = static_cast<CoreAudioManager *>(clientData);

		for (int i = 0; i < numberAddresses; i++) {
			switch (addresses[i].mSelector) {
				case kAudioHardwarePropertyDevices:
				{
					manager->onDevicesChanged();
					break;
				}
					
				case kAudioHardwarePropertyDefaultInputDevice:
				{
					PAudioCaptureDevice def = manager->getDefaultCaptureDevice();
					manager->onDefaultDeviceChanged<PAudioCaptureDevice>(kAudioObjectPropertyScopeInput, manager->captureDevices, def);
					break;
				}
				
				case kAudioHardwarePropertyDefaultOutputDevice:
				{
					PAudioPlaybackDevice defp = manager->getDefaultPlaybackDevice();
					manager->onDefaultDeviceChanged<PAudioPlaybackDevice>(kAudioObjectPropertyScopeOutput, manager->playbackDevices, defp);
					break;
				}
			}
		}
		
		return noErr;
	}

}
