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

#ifndef AVDEV_WASAPI_AUDIO_MANAGER_H_
#define AVDEV_WASAPI_AUDIO_MANAGER_H_

#include <windows.h>
#include "Mmdeviceapi.h"
#include "AudioManager.h"
#include "ComPtr.h"
#include "ComInitializer.h"
#include "WasapiAudioCaptureDevice.h"
#include "WasapiAudioPlaybackDevice.h"
#include "WindowsHelper.h"

namespace avdev
{
	class WasapiAudioManager : public AudioManager, IMMNotificationClient
	{
		public:
			WasapiAudioManager();
			~WasapiAudioManager();

			std::set<PAudioCaptureDevice> getAudioCaptureDevices();
			std::set<PAudioPlaybackDevice> getAudioPlaybackDevices();

		private:
			void enumerateDevices(EDataFlow dataFlow);
			void addDevice(LPCWSTR deviceId);
			void removeDevice(LPCWSTR deviceId);
			std::shared_ptr<AudioDevice> createAudioDevice(LPCWSTR deviceId, EDataFlow * dataFlow);
			bool insertAudioDevice(std::shared_ptr<AudioDevice> device, EDataFlow dataFlow);

			template <typename T>
			void removeAudioDevice(DeviceList<T> & devices, std::string id);

			// IMMNotificationClient implementation.
			STDMETHOD_(ULONG, AddRef)();
			STDMETHOD_(ULONG, Release)();
			STDMETHOD(QueryInterface)(REFIID iid, void ** object);
			STDMETHOD(OnDefaultDeviceChanged) (EDataFlow flow, ERole role, LPCWSTR deviceId);
			STDMETHOD(OnDeviceAdded) (LPCWSTR deviceId);
			STDMETHOD(OnDeviceRemoved) (LPCWSTR deviceId);
			STDMETHOD(OnDeviceStateChanged) (LPCWSTR deviceId, DWORD newState);
			STDMETHOD(OnPropertyValueChanged) (LPCWSTR /*deviceId*/, const PROPERTYKEY /*key*/);

			jni::ComInitializer initializer;
			jni::ComPtr<IMMDeviceEnumerator> deviceEnumerator;
	};
}

#endif