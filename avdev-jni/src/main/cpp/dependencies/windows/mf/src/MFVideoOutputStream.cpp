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

#include "MFVideoOutputStream.h"
#include "MFTypeConverter.h"
#include "MFUtils.h"
#include "MFInitializer.h"
#include "WindowsHelper.h"

namespace avdev
{
	MFVideoOutputStream::MFVideoOutputStream(std::string symbolicLink, PVideoSink sink) :
		VideoOutputStream(sink),
		symbolicLink(symbolicLink)
	{
	}

	MFVideoOutputStream::~MFVideoOutputStream()
	{
	}

	void MFVideoOutputStream::setPictureFormat(PictureFormat format)
	{
		if (!mediaSource) {
			VideoOutputStream::setPictureFormat(format);
			return;
		}

		MFInitializer initializer;
		jni::ComPtr<IMFMediaTypeHandler> typeHandler;
		jni::ComPtr<IMFMediaType> type;
		jni::ComPtr<IMFMediaType> sourceType;

		DWORD typeCount = 0;
		float frameRate = 0;
		
		HRESULT hr;

		mmf::GetMediaTypeHandler(mediaSource, 0, &typeHandler);

		hr = typeHandler->GetMediaTypeCount(&typeCount);
		THROW_IF_FAILED(hr, "MMF: Get media type count failed.");

		for (DWORD i = 0; i < typeCount; i++) {
			UINT32 width, height;

			hr = typeHandler->GetMediaTypeByIndex(i, &type);
			THROW_IF_FAILED(hr, "MMF: Get media type by index failed.");

			hr = MFGetAttributeSize(type, MF_MT_FRAME_SIZE, &width, &height);
			THROW_IF_FAILED(hr, "MMF: Get frame size failed.");

			if (width == format.getWidth() && height == format.getHeight()) {
				UINT32 num, den;
				hr = MFGetAttributeRatio(type, MF_MT_FRAME_RATE, &num, &den);
				THROW_IF_FAILED(hr, "MMF: Get frame rate failed.");

				float typeFrameRate = 1.F * num / den;

				// Pick the media type with the highest frame rate.
				if (typeFrameRate > frameRate) {
					sourceType.Attach(type);
					sourceType->AddRef();

					frameRate = typeFrameRate;
				}
			}
		}

		hr = typeHandler->SetCurrentMediaType(sourceType);
		THROW_IF_FAILED(hr, "MMF: Set current media type failed.");
	}

	void MFVideoOutputStream::setFrameRate(float frameRate)
	{
		if (!mediaSource) {
			VideoOutputStream::setFrameRate(frameRate);
			return;
		}

		MFInitializer initializer;
		jni::ComPtr<IMFMediaTypeHandler> typeHandler;
		jni::ComPtr<IMFMediaType> type;
		HRESULT hr;

		mmf::GetMediaTypeHandler(mediaSource, 0, &typeHandler);

		hr = typeHandler->GetCurrentMediaType(&type);
		THROW_IF_FAILED(hr, "MMF: Get current media type failed.");

		UINT32 num = static_cast<UINT32>(frameRate);

		MFSetAttributeRatio(type, MF_MT_FRAME_RATE, num, 1);
		MFSetAttributeRatio(type, MF_MT_FRAME_RATE_RANGE_MAX, num, 1);
		MFSetAttributeRatio(type, MF_MT_FRAME_RATE_RANGE_MIN, num, 1);

		hr = typeHandler->SetCurrentMediaType(type);
		THROW_IF_FAILED(hr, "MMF: Set current media type failed.");
	}

	void MFVideoOutputStream::openInternal()
	{
		MFInitializer initializer;
		jni::ComPtr<IMFMediaType> mediaType;

		PictureFormat format = VideoOutputStream::getPictureFormat();
		float frameRate = VideoOutputStream::getFrameRate();

		// Create output media type.
		GUID subtype = MFTypeConverter::toApiType(format.getPixelFormat());
		mmf::CreateVideoMediaType(subtype, format.getWidth(), format.getHeight(), static_cast<UINT32>(frameRate), 1, &mediaType);

		MFOutputStream::createSession(symbolicLink, mediaType);
		MFOutputStream::openSession();
	}

	void MFVideoOutputStream::closeInternal()
	{
		MFOutputStream::closeSession();
	}

	void MFVideoOutputStream::startInternal()
	{
		MFOutputStream::startSession();
	}

	void MFVideoOutputStream::stopInternal()
	{
		MFOutputStream::stopSession();
	}

	void MFVideoOutputStream::onTopologyReady()
	{
	}

	void MFVideoOutputStream::processSample(const BYTE * sampleBuffer, DWORD & sampleSize)
	{
		writeVideoFrame(sampleBuffer, sampleSize);
	}
}