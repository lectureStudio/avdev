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

#include "MFStream.h"
#include "MFUtils.h"
#include "Log.h"
#include "WindowsHelper.h"
#include "MFInitializer.h"

#include <Shlwapi.h>

namespace avdev
{
	MFStream::MFStream() :
		threadHandle(nullptr),
		mmcssState(MmcssState::Unregistered)
	{
		InitializeCriticalSection(&criticalSection);
	}

	MFStream::~MFStream()
	{
		DeleteCriticalSection(&criticalSection);
	}

	HRESULT MFStream::QueryInterface(REFIID riid, void ** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(MFStream, IMFAsyncCallback),
			{ 0 }
		};
		return QISearch(this, qit, riid, ppv);
	}

	ULONG MFStream::AddRef()
	{
		return 1U;
	}

	ULONG MFStream::Release()
	{
		return 1U;
	}

	STDMETHODIMP MFStream::GetParameters(DWORD * flags, DWORD * queue)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP MFStream::Invoke(IMFAsyncResult * asyncResult)
	{
		EnterCriticalSection(&criticalSection);

		HRESULT hr;

		if (mmcssState == MmcssState::Registering) {
			hr = queueServices->EndRegisterTopologyWorkQueuesWithMMCSS(asyncResult);

			if (FAILED(hr)) {
				LOGDEV_ERROR("MMF: End register topology work queues with MMCSS failed.");
			}

			mmcssState = MmcssState::Registered;

			LOGDEV_DEBUG("Registered MMCSS.");
		}
		else if (mmcssState == MmcssState::Unregistering) {
			hr = queueServices->EndUnregisterTopologyWorkQueuesWithMMCSS(asyncResult);

			if (FAILED(hr)) {
				LOGDEV_ERROR("MMF: End unregister topology work queues with MMCSS failed.");
			}

			mmcssState = MmcssState::Unregistered;

			LOGDEV_DEBUG("Unregistered MMCSS.");
		}

		LeaveCriticalSection(&criticalSection);

		return S_OK;
	}

	void MFStream::createSession(std::string deviceId, IMFMediaType * mediaType, WCHAR * mmcssClass)
	{
		GUID majorType;
		HRESULT hr = mediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
		THROW_IF_FAILED(hr, "MMF: Get major type failed.");

		// Create the topology.
		mmf::CreateTopology(majorType, mediaSource, sinkActivate, mmcssClass, &topology);
		mmf::CreateMediaSession(&session);
	}

	void MFStream::openSession()
	{
		mmf::ListTopologyTransforms(topology);

		threadHandle = CreateThread(nullptr, 0, run, this, 0, nullptr);

		if (threadHandle == nullptr) {
			throw AVdevException("MMF: Create thread failed.");
		}

		HRESULT hr = session->SetTopology(0, topology);
		THROW_IF_FAILED(hr, "MMF: Session set topology failed.");
	}

	void MFStream::startSession()
	{
		if (session) {
			PROPVARIANT var;
			PropVariantInit(&var);

			HRESULT hr = session->Start(&GUID_NULL, &var);

			PropVariantClear(&var);

			THROW_IF_FAILED(hr, "MMF: Session start failed.");
		}
	}

	void MFStream::stopSession()
	{
		if (session) {
			session->Stop();
		}
	}

	void MFStream::closeSession()
	{
		if (session) {
			session->Close();

			if (threadHandle != nullptr) {
				WaitForSingleObject(threadHandle, INFINITE);
				CloseHandle(threadHandle);
				threadHandle = nullptr;
			}
		}

		if (mediaSource) {
			mediaSource->Shutdown();
		}

		if (session) {
			session->Shutdown();
		}
	}

	void MFStream::runSession()
	{
		jni::ComPtr<IMFMediaEvent> event;
		MediaEventType eventType;

		HRESULT hr;
		HRESULT hrStatus = S_OK;

		try {
			MFInitializer initializer;

			while (1) {
				hr = session->GetEvent(0, &event);
				THROW_IF_FAILED(hr, "MMF: Session get event failed.");

				hr = event->GetStatus(&hrStatus);
				THROW_IF_FAILED(hr, "MMF: Event get status failed.");

				THROW_IF_FAILED(hrStatus, "MMF: Session error.");

				hr = event->GetType(&eventType);
				THROW_IF_FAILED(hr, "MMF: Event get type failed.");

				if (eventType == MESessionTopologyStatus) {
					UINT32 status = 0;

					hr = event->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &status);
					THROW_IF_FAILED(hr, "MMF: Event get topology status failed.");

					if (status == MF_TOPOSTATUS_READY) {
						registerWithMMCSS();
						onTopologyReady();
					}
				}
				else if (eventType == MESessionClosed) {
					unregisterWithMMCSS();
					break;
				}
				else if (eventType == MESessionEnded) {
					break;
				}

				event->Release();
			}
		}
		catch (AVdevException & e) {
			LOGDEV_ERROR(e.what());
		}
	}

	void MFStream::registerWithMMCSS()
	{
		if (mmcssState == MmcssState::Registering || mmcssState == MmcssState::Registered) {
			return;
		}

		EnterCriticalSection(&criticalSection);

		LOGDEV_DEBUG("Registering with MMCSS.");

		HRESULT hr = MFGetService(session, MF_WORKQUEUE_SERVICES, IID_PPV_ARGS(&queueServices));
		THROW_IF_FAILED(hr, "MMF: Get work queue services failed.");

		hr = queueServices->BeginRegisterTopologyWorkQueuesWithMMCSS(this, nullptr);
		THROW_IF_FAILED(hr, "MMF: Begin register topology work queues with MMCSS failed.");

		mmcssState = MmcssState::Registering;

		LeaveCriticalSection(&criticalSection);
	}

	void MFStream::unregisterWithMMCSS()
	{
		if (mmcssState == MmcssState::Unregistering || mmcssState == MmcssState::Unregistered) {
			return;
		}

		EnterCriticalSection(&criticalSection);

		LOGDEV_DEBUG("Unregistering with MMCSS.");

		HRESULT hr = queueServices->BeginUnregisterTopologyWorkQueuesWithMMCSS(this, nullptr);
		THROW_IF_FAILED(hr, "MMF: Begin unregister topology work queues with MMCSS failed.");

		mmcssState = MmcssState::Unregistering;

		LeaveCriticalSection(&criticalSection);
	}

	DWORD WINAPI MFStream::run(void * context)
	{
		MFStream * stream = static_cast<MFStream *>(context);
		stream->runSession();

		return 0;
	}
}