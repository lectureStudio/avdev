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

#include "MFVideoCaptureDevice.h"
#include "MFVideoOutputStream.h"
#include "MFTypeConverter.h"
#include "MFInitializer.h"
#include "WindowsHelper.h"

#include <algorithm>
#include <Mfreadwrite.h>
#include <Mferror.h>

namespace avdev {

	MFVideoCaptureDevice::MFVideoCaptureDevice(std::string name, std::string descriptor) :
		VideoCaptureDevice(name, descriptor)
	{
		MFInitializer initializer;
		jni::ComPtr<IMFMediaSource> mediaSource;
		mmf::CreateMediaSource(MFMediaType_Video, getDescriptor(), &mediaSource);

		mediaSource->QueryInterface(IID_IAMVideoProcAmp, (void**)&procAmp);
		mediaSource->QueryInterface(IID_IAMCameraControl, (void**)&cameraControl);
	}

	MFVideoCaptureDevice::~MFVideoCaptureDevice()
	{
	}

	std::list<PictureFormat> MFVideoCaptureDevice::getPictureFormats()
	{
		// Check, if formats already loaded.
		if (!formats.empty()) {
			return formats;
		}

		MFInitializer initializer;
		jni::ComPtr<IMFMediaSource> mediaSource;
		jni::ComPtr<IMFSourceReader> sourceReader;
		jni::ComPtr<IMFMediaType> type;
		DWORD typeIndex = 0;
		HRESULT hr;

		mmf::CreateMediaSource(MFMediaType_Video, getDescriptor(), &mediaSource);

		hr = MFCreateSourceReaderFromMediaSource(mediaSource, nullptr, &sourceReader);
		THROW_IF_FAILED(hr, "MMF: Create source reader from media source failed.");

		while (SUCCEEDED(hr)) {
			hr = sourceReader->GetNativeMediaType(0, typeIndex, &type);

			if (hr == MF_E_NO_MORE_TYPES) {
				break;
			}
			else if (SUCCEEDED(hr)) {
				UINT32 width, height;
				GUID guid;

				hr = MFGetAttributeSize(type, MF_MT_FRAME_SIZE, &width, &height);
				THROW_IF_FAILED(hr, "MMF: Get frame size failed.");

				hr = type->GetGUID(MF_MT_SUBTYPE, &guid);
				THROW_IF_FAILED(hr, "MMF: Get type guid failed.");

				formats.push_back(PictureFormat(width, height, MFTypeConverter::toPixelFormat(guid)));
			}

			++typeIndex;
		}

		formats.unique();

		return formats;
	}

	std::list<CameraControl> MFVideoCaptureDevice::getCameraControls()
	{
		// Check, if controls already loaded.
		if (!cameraControls.empty()) {
			return cameraControls;
		}
		if (!cameraControl) {
			// The device does not support IAMCameraControl.
			return cameraControls;
		}

		const MFTypeConverter::CameraControlMap controlMap = MFTypeConverter::getCameraControlMap();
		HRESULT hr;

		for (auto const & kv : controlMap) {
			long min, max, delta, def, flags;

			hr = cameraControl->GetRange(kv.first, &min, &max, &delta, &def, &flags);

			if (FAILED(hr) || min == max) {
				continue;
			}

			bool autoMode = (flags & KSPROPERTY_CAMERACONTROL_FLAGS_AUTO);

			cameraControls.push_back(CameraControl(kv.second, min, max, delta, def, autoMode));
		}

		return cameraControls;
	}

	std::list<PictureControl> MFVideoCaptureDevice::getPictureControls()
	{
		// Check, if controls already loaded.
		if (!pictureControls.empty()) {
			return pictureControls;
		}
		if (!procAmp) {
			// The device does not support IAMVideoProcAmp.
			return pictureControls;
		}

		const MFTypeConverter::PictureControlMap controlMap = MFTypeConverter::getPictureControlMap();
		HRESULT hr;

		for (auto const & kv : controlMap) {
			long min, max, delta, def, flags;

			hr = procAmp->GetRange(kv.first, &min, &max, &delta, &def, &flags);

			if (FAILED(hr) || min == max) {
				continue;
			}

			bool autoMode = (flags & KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO);

			pictureControls.push_back(PictureControl(kv.second, min, max, delta, def, autoMode));
		}

		return pictureControls;
	}

	void MFVideoCaptureDevice::setPictureControlAutoMode(PictureControlType type, bool autoMode)
	{
		if (!procAmp) {
			return;
		}

		long value = 0;
		long flags = 0;

		HRESULT hr = procAmp->Get(MFTypeConverter::toApiType(type), &value, &flags);
		THROW_IF_FAILED(hr, "MMF: Get picture control value failed.");

		flags = autoMode ? KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO : KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;

		hr = procAmp->Set(MFTypeConverter::toApiType(type), value, flags);
		THROW_IF_FAILED(hr, "MMF: Set picture control value failed.");
	}

	bool MFVideoCaptureDevice::getPictureControlAutoMode(PictureControlType type)
	{
		long value = getPictureControlValue(type);
		return value != 0;
	}

	void MFVideoCaptureDevice::setPictureControlValue(PictureControlType type, long value)
	{
		if (!procAmp) {
			return;
		}

		long min, max, delta, def, flags;

		HRESULT hr = procAmp->GetRange(MFTypeConverter::toApiType(type), &min, &max, &delta, &def, &flags);

		if (FAILED(hr)) {
			return;
		}

		// Respect the device range values.
		value = (std::max)(value, min);
		value = (std::min)(value, max);

		hr = procAmp->Set(MFTypeConverter::toApiType(type), value, flags);
		THROW_IF_FAILED(hr, "MMF: Set picture control value failed.");
	}

	long MFVideoCaptureDevice::getPictureControlValue(PictureControlType type)
	{
		if (!procAmp) {
			return 0;
		}

		long value = 0;
		long flags = 0;

		HRESULT hr = procAmp->Get(MFTypeConverter::toApiType(type), &value, &flags);
		THROW_IF_FAILED(hr, "MMF: Get picture control value failed.");

		return value;
	}

	void MFVideoCaptureDevice::setCameraControlAutoMode(CameraControlType type, bool autoMode)
	{
		if (!cameraControl) {
			return;
		}

		long value = 0;
		long flags = 0;

		HRESULT hr = cameraControl->Get(MFTypeConverter::toApiType(type), &value, &flags);
		THROW_IF_FAILED(hr, "MMF: Get camera control value failed.");

		flags = autoMode ? KSPROPERTY_CAMERACONTROL_FLAGS_AUTO : KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;

		hr = cameraControl->Set(MFTypeConverter::toApiType(type), value, flags);
		THROW_IF_FAILED(hr, "MMF: Set camera control value failed.");
	}

	bool MFVideoCaptureDevice::getCameraControlAutoMode(CameraControlType type)
	{
		long value = getCameraControlValue(type);
		return value != 0;
	}

	void MFVideoCaptureDevice::setCameraControlValue(CameraControlType type, long value)
	{
		if (!cameraControl) {
			return;
		}

		long min, max, delta, def, flags;

		HRESULT hr = cameraControl->GetRange(MFTypeConverter::toApiType(type), &min, &max, &delta, &def, &flags);

		if (FAILED(hr)) {
			return;
		}

		// Respect the device range values.
		value = (std::max)(value, min);
		value = (std::min)(value, max);

		hr = cameraControl->Set(MFTypeConverter::toApiType(type), value, KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);
		THROW_IF_FAILED(hr, "MMF: Set camera control value failed.");
	}

	long MFVideoCaptureDevice::getCameraControlValue(CameraControlType type)
	{
		if (!cameraControl) {
			return 0;
		}

		long value = 0;
		long flags = 0;

		HRESULT hr = cameraControl->Get(MFTypeConverter::toApiType(type), &value, &flags);
		THROW_IF_FAILED(hr, "MMF: Get camera control value failed.");

		return value;
	}

	void MFVideoCaptureDevice::setPictureFormat(PictureFormat format)
	{
		this->format = format;
	}

	PictureFormat const& MFVideoCaptureDevice::getPictureFormat() const
	{
		return format;
	}

	void MFVideoCaptureDevice::setFrameRate(float frameRate)
	{
		this->frameRate = frameRate;
	}

	float MFVideoCaptureDevice::getFrameRate() const
	{
		return frameRate;
	}

	PVideoOutputStream MFVideoCaptureDevice::createOutputStream(PVideoSink sink)
	{
		PVideoOutputStream stream = std::make_unique<MFVideoOutputStream>(getDescriptor(), sink);
		stream->setFrameRate(getFrameRate());
		stream->setPictureFormat(getPictureFormat());

		return stream;
	}

}