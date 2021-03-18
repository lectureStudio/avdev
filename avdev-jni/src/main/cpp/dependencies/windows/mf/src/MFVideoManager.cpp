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

#include "MFVideoManager.h"
#include "MFInitializer.h"
#include "WindowsHelper.h"
#include "ComPtr.h"

#include <algorithm>
#include <Ksmedia.h>

namespace avdev
{
	MFVideoManager::MFVideoManager() :
		WinHotplugNotifier(std::list<GUID> { KSCATEGORY_VIDEO })
	{
		start();
	}

	MFVideoManager::~MFVideoManager()
	{
	}

	std::set<PVideoCaptureDevice> MFVideoManager::getVideoCaptureDevices()
	{
		if (captureDevices.empty()) {
			enumerateDevices(nullptr);
		}

		return captureDevices.devices();
	}

	void MFVideoManager::enumerateDevices(std::wstring * symLink)
	{
		MFInitializer initializer;
		jni::ComPtr<IMFAttributes> pAttributes;
		UINT32 deviceCount = 0;
		IMFActivate ** ppDevices;
		HRESULT hr;

		hr = MFCreateAttributes(&pAttributes, 1);
		THROW_IF_FAILED(hr, "MMF: Create attributes failed.");

		hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
		THROW_IF_FAILED(hr, "MMF: Set video source GUID failed.");

		hr = MFEnumDeviceSources(pAttributes, &ppDevices, &deviceCount);
		THROW_IF_FAILED(hr, "MMF: Enumerate video sources failed.");

		for (DWORD i = 0; i < deviceCount; i++) {
			WCHAR * friendlyName = nullptr;
			WCHAR * symbolicLink = nullptr;

			hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &friendlyName, nullptr);
			if (FAILED(hr)) {
				LOGDEV_ERROR("MMF: Get video source name failed: hr = 0x%08x.", hr);
				continue;
			}
			hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &symbolicLink, nullptr);
			if (FAILED(hr)) {
				LOGDEV_ERROR("MMF: Get video source symbolic link failed: hr = 0x%08x.", hr);
				continue;
			}

			if (symLink == nullptr) {
				std::shared_ptr<VideoDevice> device = createVideoDevice(symbolicLink, friendlyName);
				insertVideoDevice(device);
			}
			else {
				// IMFActivate and the device broadcaster return different
				// symbolic links. Compare only the device instance id.
				std::wstring link(symbolicLink);
				size_t pos = link.find(L"#{", 0);
				pos = (pos == -1) ? (std::min)(link.length(), (*symLink).length()) : pos;

				if ((*symLink).compare(0, pos, link, 0, pos) == 0) {
					std::shared_ptr<VideoDevice> device = createVideoDevice(symbolicLink, friendlyName);

					if (insertVideoDevice(device)) {
						notifyDeviceConnected(device);
					}
					break;
				}
			}

			CoTaskMemFree(friendlyName);
			CoTaskMemFree(symbolicLink);

			ppDevices[i]->Release();
		}
		
		if (deviceCount != 0) {
			CoTaskMemFree(ppDevices);
			ppDevices = nullptr;
		}
	}

	std::shared_ptr<VideoDevice> MFVideoManager::createVideoDevice(WCHAR * symbolicLink, WCHAR * friendlyName)
	{
		std::string id = WideStrToStr(symbolicLink);
		std::string name = WideStrToStr(friendlyName);

		std::shared_ptr<VideoDevice> device = nullptr;

		device = std::make_shared<MFVideoCaptureDevice>(name, id);

		return device;
	}

	bool MFVideoManager::insertVideoDevice(std::shared_ptr<VideoDevice> device)
	{
		if (device == nullptr) {
			return false;
		}

		captureDevices.insertDevice(std::static_pointer_cast<MFVideoCaptureDevice>(device));

		return true;
	}

	void MFVideoManager::onDeviceConnected(std::wstring symLink)
	{
		enumerateDevices(&symLink);
	}

	void MFVideoManager::onDeviceDisconnected(std::wstring symLink)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

		auto predicate = [&converter, symLink](const PVideoCaptureDevice & dev) {
			std::wstring link = converter.from_bytes(dev->getDescriptor());
			std::transform(link.begin(), link.end(), link.begin(), ::tolower);

			// IMFActivate and the device broadcaster return different
			// symbolic links. Compare only the device instance id.
			size_t pos = link.find(L"#{", 0);
			pos = (pos == -1) ? (std::min)(link.length(), symLink.length()) : pos;

			return symLink.compare(0, pos, link, 0, pos) == 0;
		};

		PVideoCaptureDevice removed = captureDevices.removeDevice(predicate);

		if (removed) {
			notifyDeviceDisconnected(removed);
		}
	}

}