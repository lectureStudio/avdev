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

#include "WasapiAudioManager.h"
#include "Functiondiscoverykeys_devpkey.h"
#include "ComInitializer.h"
#include "Log.h"

namespace avdev
{
	WasapiAudioManager::WasapiAudioManager() :
		deviceEnumerator()
	{
		jni::ComInitializer initializer;

		HRESULT hr;
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
		THROW_IF_FAILED(hr, "WASAPI: Create device enumerator failed.");

		deviceEnumerator->RegisterEndpointNotificationCallback(this);

		getAudioCaptureDevices();
		getAudioPlaybackDevices();
	}

	WasapiAudioManager::~WasapiAudioManager()
	{
		if (deviceEnumerator) {
			deviceEnumerator->UnregisterEndpointNotificationCallback(this);
		}
	}

	std::set<PAudioCaptureDevice> WasapiAudioManager::getAudioCaptureDevices()
	{
		if (captureDevices.empty()) {
			enumerateDevices(eCapture);
		}

		return captureDevices.devices();
	}

	std::set<PAudioPlaybackDevice> WasapiAudioManager::getAudioPlaybackDevices()
	{
		if (playbackDevices.empty()) {
			enumerateDevices(eRender);
		}

		return playbackDevices.devices();
	}

	void WasapiAudioManager::enumerateDevices(EDataFlow dataFlow)
	{
		jni::ComInitializer initializer;
		jni::ComPtr<IMMDeviceCollection> deviceCollection;
		jni::ComPtr<IMMDevice> defaultDevice;
		LPWSTR defaultDeviceId = nullptr;
		UINT count;
		HRESULT hr;

		hr = deviceEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &deviceCollection);
		THROW_IF_FAILED(hr, "WASAPI: Enumerate audio endpoints failed.");

		hr = deviceCollection->GetCount(&count);
		THROW_IF_FAILED(hr, "WASAPI: Get device count failed.");

		hr = deviceEnumerator->GetDefaultAudioEndpoint(dataFlow, eMultimedia, &defaultDevice);
		if (SUCCEEDED(hr)) {
			hr = defaultDevice->GetId(&defaultDeviceId);
			THROW_IF_FAILED(hr, "WASAPI: Get default device id failed.");
		}

		for (UINT i = 0; i < count; i++) {
			LPWSTR deviceId = nullptr;
			jni::ComPtr<IMMDevice> pDevice;

			hr = deviceCollection->Item(i, &pDevice);
			if (FAILED(hr)) {
				LOGDEV_ERROR("WASAPI: DeviceCollection get device failed: hr = 0x%08x.", hr);
				continue;
			}

			hr = pDevice->GetId(&deviceId);
			if (FAILED(hr)) {
				LOGDEV_ERROR("WASAPI: Device get id failed: hr = 0x%08x.", hr);
				continue;
			}

			std::shared_ptr<AudioDevice> device = createAudioDevice(deviceId, nullptr);

			if (device != nullptr) {
				insertAudioDevice(device, dataFlow);

				bool isDefault = (defaultDeviceId != nullptr && wcscmp(defaultDeviceId, deviceId) == 0);

				if (dataFlow == eCapture && isDefault) {
					setDefaultCaptureDevice(std::static_pointer_cast<WasapiAudioCaptureDevice>(device));
				}
				else if (dataFlow == eRender && isDefault) {
					setDefaultPlaybackDevice(std::static_pointer_cast<WasapiAudioPlaybackDevice>(device));
				}
			}

			CoTaskMemFree(deviceId);
		}

		CoTaskMemFree(defaultDeviceId);
	}

	void WasapiAudioManager::addDevice(LPCWSTR deviceId)
	{
		if (deviceId == nullptr)
			return;

		EDataFlow dataFlow;
		std::shared_ptr<AudioDevice> device = createAudioDevice(deviceId, &dataFlow);
		bool inserted = insertAudioDevice(device, dataFlow);

		if (inserted) {
			notifyDeviceConnected(device);
		}
	}

	void WasapiAudioManager::removeDevice(LPCWSTR deviceId)
	{
		if (deviceId == nullptr)
			return;

		std::string id = WideStrToStr(deviceId);

		removeAudioDevice(captureDevices, id);
		removeAudioDevice(playbackDevices, id);
	}

	std::shared_ptr<AudioDevice> WasapiAudioManager::createAudioDevice(LPCWSTR deviceId, EDataFlow * dataFlow)
	{
		jni::ComInitializer initializer;
		jni::ComPtr<IMMDevice> pDevice;
		jni::ComPtr<IMMEndpoint> endpoint;
		jni::ComPtr<IPropertyStore> propertyStore;
		EDataFlow flow;
		PROPVARIANT pv;
		PropVariantInit(&pv);
		HRESULT hr;

		hr = deviceEnumerator->GetDevice(deviceId, &pDevice);
		THROW_IF_FAILED(hr, "WASAPI: Enumerator get device with id: %S failed.", deviceId);

		hr = pDevice->QueryInterface(__uuidof(IMMEndpoint), (void**)&endpoint);
		THROW_IF_FAILED(hr, "WASAPI: Device get endpoint failed.");

		hr = endpoint->GetDataFlow(&flow);
		THROW_IF_FAILED(hr, "WASAPI: Endpoint get data flow failed.");

		hr = pDevice->OpenPropertyStore(STGM_READ, &propertyStore);
		THROW_IF_FAILED(hr, "WASAPI: Device open property store failed.");

		hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
		THROW_IF_FAILED(hr, "WASAPI: PropertyStore get friendly name failed.");

		std::string id = WideStrToStr(deviceId);
		std::string name = WideStrToStr(pv.pwszVal);
		std::wstring devId = std::wstring(deviceId);

		std::shared_ptr<AudioDevice> device = nullptr;

		if (flow == eCapture) {
			device = std::make_shared<WasapiAudioCaptureDevice>(name, id, devId);
		}
		else if (flow == eRender) {
			device = std::make_shared<WasapiAudioPlaybackDevice>(name, id, devId);
		}

		if (dataFlow != nullptr)
			*dataFlow = flow;

		PropVariantClear(&pv);

		return device;
	}

	bool WasapiAudioManager::insertAudioDevice(std::shared_ptr<AudioDevice> device, EDataFlow dataFlow)
	{
		if (device == nullptr)
			return false;

		if (dataFlow == eCapture) {
			captureDevices.insertDevice(std::static_pointer_cast<WasapiAudioCaptureDevice>(device));
			return true;
		}
		else if (dataFlow == eRender) {
			playbackDevices.insertDevice(std::static_pointer_cast<WasapiAudioPlaybackDevice>(device));
			return true;
		}

		return false;
	}

	template <typename T>
	void WasapiAudioManager::removeAudioDevice(DeviceList<T> & devices, std::string id)
	{
		auto predicate = [id](const std::shared_ptr<AudioDevice> & dev) {
			return id == dev->getDescriptor();
		};

		T removed = devices.removeDevice(predicate);

		if (removed) {
			notifyDeviceDisconnected(removed);
		}
	}

	ULONG WasapiAudioManager::AddRef()
	{
		// We hold a reference to ourselves (static singleton).
		return 1;
	}

	ULONG WasapiAudioManager::Release()
	{
		// We hold a reference to ourselves (static singleton).
		return 1;
	}

	HRESULT WasapiAudioManager::QueryInterface(REFIID iid, void ** object)
	{
		if (object == nullptr) {
			return E_POINTER;
		}

		if (iid == IID_IUnknown || iid == __uuidof(IMMNotificationClient)) {
			*object = static_cast<IMMNotificationClient *>(this);
		}
		else {
			*object = nullptr;
			return E_NOINTERFACE;
		}

		AddRef();

		return S_OK;
	}

	HRESULT WasapiAudioManager::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR deviceId)
	{
		LOGDEV_INFO("WASAPI: Default device changed (%s): %S", RoleToStr(role).c_str(), deviceId);

		std::string id;

		if (deviceId != nullptr) {
			id = WideStrToStr(deviceId);
		}

		auto predicate = [id](const std::shared_ptr<AudioDevice> & dev) {
			return id == dev->getDescriptor();
		};

		if (flow == eCapture) {
			if (deviceId == nullptr) {
				setDefaultCaptureDevice(nullptr);
				return S_OK;
			}

			PAudioCaptureDevice found = captureDevices.findDevice(predicate);

			if (found) {
				setDefaultCaptureDevice(found);
			}
		}
		else if (flow == eRender) {
			if (deviceId == nullptr) {
				setDefaultPlaybackDevice(nullptr);
				return S_OK;
			}

			PAudioPlaybackDevice found = playbackDevices.findDevice(predicate);

			if (found) {
				setDefaultPlaybackDevice(found);
			}
		}

		return S_OK;
	}

	HRESULT WasapiAudioManager::OnDeviceAdded(LPCWSTR deviceId)
	{
		LOGDEV_INFO("WASAPI: Device added: %S", deviceId);

		addDevice(deviceId);

		return S_OK;
	}

	HRESULT WasapiAudioManager::OnDeviceRemoved(LPCWSTR deviceId)
	{
		LOGDEV_INFO("WASAPI: Device removed: %S", deviceId);

		removeDevice(deviceId);

		return S_OK;
	}

	HRESULT WasapiAudioManager::OnDeviceStateChanged(LPCWSTR deviceId, DWORD newState)
	{
		LOGDEV_INFO("WASAPI: Device state changed: %S", deviceId);

		if (deviceId == nullptr)
			return S_OK;

		switch (newState) {
			case DEVICE_STATE_ACTIVE:
				addDevice(deviceId);
				break;

			case DEVICE_STATE_DISABLED:
			case DEVICE_STATE_NOTPRESENT:
			case DEVICE_STATE_UNPLUGGED:
				removeDevice(deviceId);
				break;
		}

		return S_OK;
	}

	HRESULT WasapiAudioManager::OnPropertyValueChanged(LPCWSTR /*deviceId*/, const PROPERTYKEY /*key*/)
	{
		return S_OK;
	}
}