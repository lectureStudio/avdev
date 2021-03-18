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

#include "MFOutputStream.h"
#include "MFUtils.h"
#include "WindowsHelper.h"

#include <Shlwapi.h>

namespace avdev
{
	MFOutputStream::MFOutputStream()
	{
	}

	MFOutputStream::~MFOutputStream()
	{
	}

	void MFOutputStream::createSession(std::string deviceId, IMFMediaType * mediaType)
	{
		GUID majorType;
		HRESULT hr = mediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
		THROW_IF_FAILED(hr, "MMF: Get major type failed.");

		// Create the topology.
		mmf::CreateMediaSource(majorType, deviceId, &mediaSource);
		mmf::CreateSinkActivate(this, mediaType, &sinkActivate);
		mmf::CreateTopology(majorType, mediaSource, sinkActivate, L"Capture", &topology);
		mmf::CreateMediaSession(&session);
	}

	HRESULT MFOutputStream::QueryInterface(REFIID riid, void ** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(MFOutputStream, IMFSampleGrabberSinkCallback),
			QITABENT(MFOutputStream, IMFClockStateSink),
			QITABENT(MFOutputStream, IMFAsyncCallback),
			{ 0 }
		};
		return QISearch(this, qit, riid, ppv);
	}

	ULONG MFOutputStream::AddRef()
	{
		return 1U;
	}

	ULONG MFOutputStream::Release()
	{
		return 1U;
	}

	STDMETHODIMP MFOutputStream::OnClockStart(MFTIME systemTime, LONGLONG clockStartOffset)
	{
		return S_OK;
	}

	STDMETHODIMP MFOutputStream::OnClockStop(MFTIME systemTime)
	{
		return S_OK;
	}

	STDMETHODIMP MFOutputStream::OnClockPause(MFTIME systemTime)
	{
		return S_OK;
	}

	STDMETHODIMP MFOutputStream::OnClockRestart(MFTIME systemTime)
	{
		return S_OK;
	}

	STDMETHODIMP MFOutputStream::OnClockSetRate(MFTIME systemTime, float rate)
	{
		return S_OK;
	}

	STDMETHODIMP MFOutputStream::OnSetPresentationClock(IMFPresentationClock * clock)
	{
		return S_OK;
	}

	STDMETHODIMP MFOutputStream::OnProcessSample(REFGUID majorMediaType, DWORD sampleFlags, LONGLONG sampleTime, LONGLONG sampleDuration, const BYTE * sampleBuffer, DWORD sampleSize)
	{
		//printf("%lld %lld %d \n", sampleTime, sampleDuration, sampleSize);
		//fflush(NULL);
		processSample(sampleBuffer, sampleSize);
		return S_OK;
	}

	STDMETHODIMP MFOutputStream::OnShutdown()
	{
		return S_OK;
	}
}