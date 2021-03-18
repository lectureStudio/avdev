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

#include "AVdevException.h"
#include "MFPcmMediaSource.h"
#include "MFUtils.h"
#include "WindowsHelper.h"
#include "ComPtr.h"

#include <Mferror.h>
#include <Shlwapi.h>

namespace avdev
{
	MFPcmMediaSource::MFPcmMediaSource(PAudioSource source, IMFMediaType * mediaType, HRESULT & hr) :
		source(source),
		mediaType(mediaType),
		refCount(1),
		shutdown(FALSE),
		state(State::STOPPED)
	{
		// Create the media event queue.
		hr = MFCreateEventQueue(&eventQueue);

		InitializeCriticalSection(&criticalSection);
	}

	MFPcmMediaSource::~MFPcmMediaSource()
	{
		DeleteCriticalSection(&criticalSection);
	}

	HRESULT MFPcmMediaSource::CreateInstance(PAudioSource audioSource, IMFMediaType * mediaType, REFIID iid, void ** source)
	{
		if (!source) {
			return E_POINTER;
		}

		HRESULT hr = S_OK;

		jni::ComPtr<MFPcmMediaSource> pSource = new (std::nothrow) MFPcmMediaSource(audioSource, mediaType, hr);

		if (!pSource) {
			return E_OUTOFMEMORY;
		}

		if (SUCCEEDED(hr)) {
			hr = pSource->QueryInterface(iid, source);
		}

		return hr;
	}

	STDMETHODIMP MFPcmMediaSource::QueryInterface(REFIID iid, void ** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(MFPcmMediaSource, IMFMediaEventGenerator),
			QITABENT(MFPcmMediaSource, IMFMediaSource),
			{ 0 }
		};
		return QISearch(this, qit, iid, ppv);
	}
	
	STDMETHODIMP_(ULONG) MFPcmMediaSource::AddRef()
	{
		return InterlockedIncrement(&refCount);
	}
	
	STDMETHODIMP_(ULONG) MFPcmMediaSource::Release()
	{
		ULONG count = InterlockedDecrement(&refCount);
		if (count == 0) {
			delete this;
		}

		// For thread safety, return a temporary variable.
		return count;
	}
	
	STDMETHODIMP MFPcmMediaSource::BeginGetEvent(IMFAsyncCallback * callback, IUnknown * punkState)
	{
		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			hr = eventQueue->BeginGetEvent(callback, punkState);
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}
	
	STDMETHODIMP MFPcmMediaSource::EndGetEvent(IMFAsyncResult * result, IMFMediaEvent ** mediaEvent)
	{
		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			hr = eventQueue->EndGetEvent(result, mediaEvent);
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}
	
	STDMETHODIMP MFPcmMediaSource::GetEvent(DWORD flags, IMFMediaEvent ** mediaEvent)
	{
		jni::ComPtr<IMFMediaEventQueue> queue;

		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			queue = eventQueue;
			queue->AddRef();
		}

		LeaveCriticalSection(&criticalSection);

		// GetEvent can block indefinitely, so we don't hold the MFPcmMediaSource lock.
		if (SUCCEEDED(hr)) {
			hr = queue->GetEvent(flags, mediaEvent);
		}

		return hr;
	}
	
	STDMETHODIMP MFPcmMediaSource::QueueEvent(MediaEventType eventType, REFGUID extendedType, HRESULT status, const PROPVARIANT * value)
	{
		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			hr = eventQueue->QueueEventParamVar(eventType, extendedType, status, value);
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}
	
	STDMETHODIMP MFPcmMediaSource::CreatePresentationDescriptor(IMFPresentationDescriptor ** presentationDescriptor)
	{
		if (!presentationDescriptor) {
			return E_POINTER;
		}

		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr) && !this->presentationDescriptor) {
			CreatePresentationDescriptor();
		}

		if (SUCCEEDED(hr)) {
			hr = this->presentationDescriptor->Clone(presentationDescriptor);
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}
	
	STDMETHODIMP MFPcmMediaSource::GetCharacteristics(DWORD * characteristics)
	{
		if (!characteristics) {
			return E_POINTER;
		}

		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			*characteristics = MFMEDIASOURCE_CAN_PAUSE | MFMEDIASOURCE_CAN_SEEK;
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}
	
	STDMETHODIMP MFPcmMediaSource::Pause()
	{
		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		// Pause is only allowed from started state.
		if (SUCCEEDED(hr)) {
			if (state != State::STARTED) {
				hr = MF_E_INVALID_STATE_TRANSITION;
			}
		}

		// Send the appropriate events.
		if (SUCCEEDED(hr)) {
			if (stream) {
				hr = stream->QueueEvent(MEStreamPaused, GUID_NULL, S_OK, nullptr);
			}
		}

		if (SUCCEEDED(hr)) {
			hr = QueueEvent(MESourcePaused, GUID_NULL, S_OK, nullptr);
		}

		if (SUCCEEDED(hr)) {
			state = State::PAUSED;
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}
	
	STDMETHODIMP MFPcmMediaSource::Shutdown()
	{
		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			if (stream) {
				stream->Shutdown();
			}

			if (eventQueue) {
				eventQueue->Shutdown();
			}

			shutdown = TRUE;
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}
	
	STDMETHODIMP MFPcmMediaSource::Start(IMFPresentationDescriptor * presentationDescriptor, const GUID * timeFormat, const PROPVARIANT * startPosition)
	{
		LONGLONG startOffset = 0;
		BOOL seek = FALSE;
		BOOL restartFromCurrentPosition = FALSE;
		BOOL queuedStartEvent = FALSE;
		HRESULT hr;

		jni::ComPtr<IMFMediaEvent> event;

		PROPVARIANT var;
		PropVariantInit(&var);

		if (!startPosition || !presentationDescriptor) {
			return E_INVALIDARG;
		}

		if ((timeFormat != NULL) && (*timeFormat != GUID_NULL)) {
			// Unrecognized time format GUID.
			return MF_E_UNSUPPORTED_TIME_FORMAT;
		}

		EnterCriticalSection(&criticalSection);

		hr = CheckShutdown();

		if (FAILED(hr)) { goto done; }

		if (startPosition->vt == VT_I8) {
			// Start position is given in pvarStartPosition in 100-ns units.
			startOffset = startPosition->hVal.QuadPart;

			if (state != State::STOPPED) {
				seek = TRUE;
			}
		}
		else if (startPosition->vt == VT_EMPTY) {
			if (state == State::STOPPED) {
				startOffset = 0;
			}
			else {
				startOffset = GetCurrentPosition();
				restartFromCurrentPosition = TRUE;
			}
		}
		else {
			hr = MF_E_UNSUPPORTED_TIME_FORMAT;
			goto done;
		}

		ValidatePresentationDescriptor(presentationDescriptor);

		// Sends the MENewStream or MEUpdatedStream event.
		QueueNewStreamEvent(presentationDescriptor);

		hr = stream->SetPosition(startOffset);

		if (FAILED(hr)) { goto done; }

		var.vt = VT_I8;
		var.hVal.QuadPart = startOffset;

		if (seek) {
			hr = QueueEvent(MESourceSeeked, GUID_NULL, hr, &var);

			if (FAILED(hr)) { goto done; }
		}
		else {
			hr = MFCreateMediaEvent(MESourceStarted, GUID_NULL, hr, &var, &event);

			if (FAILED(hr)) { goto done; }

			if (restartFromCurrentPosition) {
				hr = event->SetUINT64(MF_EVENT_SOURCE_ACTUAL_START, startOffset);
				if (FAILED(hr)) { goto done; }
			}

			hr = eventQueue->QueueEvent(event);

			if (FAILED(hr)) { goto done; }
		}

		queuedStartEvent = TRUE;

		// Send the stream event.
		if (stream) {
			if (seek) {
				hr = stream->QueueEvent(MEStreamSeeked, GUID_NULL, hr, &var);
			}
			else {
				hr = stream->QueueEvent(MEStreamStarted, GUID_NULL, hr, &var);
			}

			if (FAILED(hr)) { goto done; }
		}

		if (seek) {
			stream->Flush();
		}
		else {
			stream->DeliverQueuedSamples();
		}

		if (FAILED(hr)) { goto done; }

		state = State::STARTED;

	done:
		// If a failure occurred and we have already sent the 
		// event (with a success code), then we need to raise an
		// MEError event.

		if (FAILED(hr) && queuedStartEvent) {
			hr = QueueEvent(MEError, GUID_NULL, hr, &var);
		}

		PropVariantClear(&var);

		LeaveCriticalSection(&criticalSection);

		return hr;
	}
	
	STDMETHODIMP MFPcmMediaSource::Stop()
	{
		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			state = State::STOPPED;

			// Flush all queued samples.
			stream->Flush();
		}

		if (SUCCEEDED(hr)) {
			if (stream) {
				hr = stream->QueueEvent(MEStreamStopped, GUID_NULL, S_OK, nullptr);
			}
		}
		if (SUCCEEDED(hr)) {
			hr = QueueEvent(MESourceStopped, GUID_NULL, S_OK, nullptr);
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}
	
	void MFPcmMediaSource::CreatePresentationDescriptor()
	{
		jni::ComPtr<IMFStreamDescriptor> streamDescriptor;
		jni::ComPtr<IMFMediaTypeHandler> typeHandler;
		MFTIME duration = 0;
		HRESULT hr;

		hr = MFCreateStreamDescriptor(0, 1, &mediaType, &streamDescriptor);
		THROW_IF_FAILED(hr, "MMF: Create stream descriptor failed.");

		hr = streamDescriptor->GetMediaTypeHandler(&typeHandler);
		THROW_IF_FAILED(hr, "MMF: Get media type handler failed.");

		hr = typeHandler->SetCurrentMediaType(mediaType);
		THROW_IF_FAILED(hr, "MMF: Set current media type failed.");

		hr = MFCreatePresentationDescriptor(1, &streamDescriptor, &presentationDescriptor);
		THROW_IF_FAILED(hr, "MMF: Create presentation descriptor failed.");

		hr = presentationDescriptor->SelectStream(0);
		THROW_IF_FAILED(hr, "MMF: Select stream %d failed.", 0);

		//duration = 20000;
		//hr = presentationDescriptor->SetUINT64(MF_PD_DURATION, (UINT64)duration);
	}

	void MFPcmMediaSource::CreatePcmStream(IMFStreamDescriptor * sd)
	{
		HRESULT hr = S_OK;
		stream = new (std::nothrow) PcmMediaStream(source, mediaType, this, sd, hr);

		if (!stream) {
			throw AVdevException("PcmMediaSource: Stream does not exist.");
		}

		if (FAILED(hr)) {
			if (stream) {
				stream->Release();
				stream = nullptr;
			}
		}
	}

	void MFPcmMediaSource::QueueNewStreamEvent(IMFPresentationDescriptor * pd)
	{
		jni::ComPtr<IMFStreamDescriptor> sd;
		BOOL selected = FALSE;

		HRESULT hr = pd->GetStreamDescriptorByIndex(0, &selected, &sd);
		THROW_IF_FAILED(hr, "MMF: Get stream descriptor by index %d failed.", 0);

		if (!selected) {
			throw AVdevException("MMF: Stream %d is not selected.", 0);
		}

		if (stream) {
			hr = mmf::QueueEventWithIUnknown(this, MEUpdatedStream, S_OK, stream);
			THROW_IF_FAILED(hr, "MMF: Queue update stream event failed.");
		}
		else {
			CreatePcmStream(sd);
			hr = mmf::QueueEventWithIUnknown(this, MENewStream, S_OK, stream);
			THROW_IF_FAILED(hr, "MMF: Queue new stream event failed.");
		}
	}

	void MFPcmMediaSource::ValidatePresentationDescriptor(IMFPresentationDescriptor * pd)
	{
		jni::ComPtr<IMFStreamDescriptor> streamDescriptor;
		jni::ComPtr<IMFMediaTypeHandler> handler;
		jni::ComPtr<IMFMediaType> currentMediaType;
		jni::ComPtr<IMFMediaType> sourceType;

		DWORD streamDescriptors = 0;
		BOOL selected = FALSE;
		
		// Make sure there is only one stream.
		HRESULT hr = pd->GetStreamDescriptorCount(&streamDescriptors);
		THROW_IF_FAILED(hr, "MMF: Get stream descriptor count failed.");

		if (streamDescriptors != 1) {
			throw AVdevException("MMF: PcmMediaSource has to provide exactly one stream, but %ld streams found.", streamDescriptors);
		}

		hr = pd->GetStreamDescriptorByIndex(0, &selected, &streamDescriptor);
		THROW_IF_FAILED(hr, "MMF: Get stream descriptor by index %d failed.", 0);

		// Make sure it's selected.
		if (!selected) {
			throw AVdevException("MMF: Stream descriptor at index %ld is not selected.", 0);
		}

		hr = streamDescriptor->GetMediaTypeHandler(&handler);
		THROW_IF_FAILED(hr, "MMF: Get media type handler failed.");

		hr = handler->GetCurrentMediaType(&currentMediaType);
		THROW_IF_FAILED(hr, "MMF: Get current media type failed.");

		BOOL equal = FALSE;

		hr = mediaType->Compare(currentMediaType, MF_ATTRIBUTES_MATCH_ALL_ITEMS, &equal);
		THROW_IF_FAILED(hr, "MMF: Compare media types failed.");

		if (!equal) {
			THROW_IF_FAILED(hr, "MMF: Media types do not match.");
		}
	}

	LONGLONG MFPcmMediaSource::GetCurrentPosition() const
	{
		return (stream ? stream->GetCurrentPosition() : 0);
	}

	MFPcmMediaSource::State MFPcmMediaSource::GetState() const
	{
		return state;
	}

	HRESULT MFPcmMediaSource::CheckShutdown() const
	{
		return (shutdown ? MF_E_SHUTDOWN : S_OK);
	}









	PcmMediaStream::PcmMediaStream(PAudioSource audioSource, IMFMediaType * mediaType, MFPcmMediaSource * source, IMFStreamDescriptor * sd, HRESULT & hr) :
		source(audioSource),
		mediaType(mediaType),
		refCount(1),
		shutdown(FALSE),
		currentPosition(0),
		discontinuity(FALSE),
		eos(FALSE),
		mediaSource(source),
		streamDescriptor(sd)
	{
		mediaSource->AddRef();
		streamDescriptor->AddRef();

		UINT32 sampleRate;
		UINT32 channels;
		UINT32 bitsPerSample;

		hr = mediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
		THROW_IF_FAILED(hr, "MMF: Get sample rate failed.");

		hr = mediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
		THROW_IF_FAILED(hr, "MMF: Get audio channels failed.");

		hr = mediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);
		THROW_IF_FAILED(hr, "MMF: Get bits per sample failed.");

		bufferSize = static_cast<DWORD>(sampleRate * (channels * bitsPerSample / 8) * 0.01);

		// Create the media event queue.
		hr = MFCreateEventQueue(&eventQueue);

		InitializeCriticalSection(&criticalSection);
	}

	PcmMediaStream::~PcmMediaStream()
	{
		DeleteCriticalSection(&criticalSection);
	}

	HRESULT PcmMediaStream::QueryInterface(REFIID iid, void ** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(PcmMediaStream, IMFMediaEventGenerator),
			QITABENT(PcmMediaStream, IMFMediaStream),
			{ 0 }
		};
		return QISearch(this, qit, iid, ppv);
	}

	ULONG PcmMediaStream::AddRef()
	{
		return InterlockedIncrement(&refCount);
	}

	ULONG PcmMediaStream::Release()
	{
		ULONG uCount = InterlockedDecrement(&refCount);
		if (uCount == 0) {
			delete this;
		}

		// For thread safety, return a temporary variable.
		return uCount;
	}

	STDMETHODIMP PcmMediaStream::BeginGetEvent(IMFAsyncCallback * callback, IUnknown * punkState)
	{
		HRESULT hr;

		EnterCriticalSection(&criticalSection);

		hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			hr = eventQueue->BeginGetEvent(callback, punkState);
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}

	STDMETHODIMP PcmMediaStream::EndGetEvent(IMFAsyncResult * result, IMFMediaEvent ** mediaEvent)
	{
		HRESULT hr;

		EnterCriticalSection(&criticalSection);

		hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			hr = eventQueue->EndGetEvent(result, mediaEvent);
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}

	STDMETHODIMP PcmMediaStream::GetEvent(DWORD flags, IMFMediaEvent ** mediaEvent)
	{
		HRESULT hr;

		jni::ComPtr<IMFMediaEventQueue> queue;

		EnterCriticalSection(&criticalSection);

		hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			queue = eventQueue;
			queue->AddRef();
		}

		LeaveCriticalSection(&criticalSection);

		if (SUCCEEDED(hr)) {
			hr = queue->GetEvent(flags, mediaEvent);
		}

		return hr;
	}

	STDMETHODIMP PcmMediaStream::QueueEvent(MediaEventType eventType, REFGUID extendedType, HRESULT status, const PROPVARIANT * value)
	{
		HRESULT hr;

		EnterCriticalSection(&criticalSection);

		hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			hr = eventQueue->QueueEventParamVar(eventType, extendedType, status, value);
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}

	STDMETHODIMP PcmMediaStream::GetMediaSource(IMFMediaSource ** mediaSource)
	{
		if (!mediaSource) {
			return E_POINTER;
		}

		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			if (mediaSource == NULL) {
				hr = E_UNEXPECTED;
			}
		}

		if (SUCCEEDED(hr)) {
			hr = this->mediaSource->QueryInterface(IID_PPV_ARGS(mediaSource));
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}

	STDMETHODIMP PcmMediaStream::GetStreamDescriptor(IMFStreamDescriptor ** streamDescriptor)
	{
		if (!streamDescriptor) {
			return E_POINTER;
		}
		if (!streamDescriptor) {
			return E_UNEXPECTED;
		}

		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			*streamDescriptor = this->streamDescriptor;
			(*streamDescriptor)->AddRef();
		}

		LeaveCriticalSection(&criticalSection);

		return hr;
	}

	STDMETHODIMP PcmMediaStream::RequestSample(IUnknown * token)
	{
		if (!mediaSource) {
			return E_UNEXPECTED;
		}

		jni::ComPtr<IMFMediaSource> pSource;
		jni::ComPtr<IMFSample> pSample;

		EnterCriticalSection(&criticalSection);

		HRESULT hr = CheckShutdown();

		if (SUCCEEDED(hr)) {
			if (eos) {
				hr = MF_E_END_OF_STREAM;
			}
		}

		if (SUCCEEDED(hr)) {
			if (mediaSource->GetState() == MFPcmMediaSource::State::STOPPED)	{
				hr = MF_E_INVALIDREQUEST;
			}
		}

		if (SUCCEEDED(hr)) {
			hr = CreateAudioSample(&pSample);
		}

		if (SUCCEEDED(hr)) {
			if (token) {
				hr = pSample->SetUnknown(MFSampleExtension_Token, token);
			}
		}

		if (SUCCEEDED(hr))
		{
			if (mediaSource->GetState() == MFPcmMediaSource::State::PAUSED) {
				hr = sampleQueue.Queue(pSample);
			}
			else {
				hr = DeliverSample(pSample);
			}
		}

		// Cache a pointer to the source, prior to leaving the critical section.
		if (SUCCEEDED(hr)) {
			pSource.Attach(mediaSource);
			pSource->AddRef();
		}

		LeaveCriticalSection(&criticalSection);

		if (SUCCEEDED(hr)) {
			if (eos) {
				hr = pSource->QueueEvent(MEEndOfPresentation, GUID_NULL, S_OK, NULL);
			}
		}

		return hr;
	}

	HRESULT PcmMediaStream::CheckShutdown() const {
		if (shutdown) {
			return MF_E_SHUTDOWN;
		}
		else {
			return S_OK;
		}
	}

	HRESULT PcmMediaStream::Shutdown()
	{
		EnterCriticalSection(&criticalSection);

		Flush();

		if (eventQueue) {
			eventQueue->Shutdown();
		}

		eventQueue->Release();
		mediaSource->Release();
		streamDescriptor->Release();

		shutdown = TRUE;

		LeaveCriticalSection(&criticalSection);

		return S_OK;
	}

	HRESULT PcmMediaStream::CreateAudioSample(IMFSample ** sample)
	{
		jni::ComPtr<IMFMediaBuffer> buffer;
		jni::ComPtr<IMFSample> pSample;

		BYTE * bufferData = nullptr;
		LONGLONG duration = 0;
		int read = 0;

		HRESULT hr = MFCreateMemoryBuffer(bufferSize, &buffer);

		if (SUCCEEDED(hr)) {
			hr = buffer->Lock(&bufferData, NULL, NULL);
		}

		if (SUCCEEDED(hr)) {
			read = source->read(bufferData, 0, bufferSize);
		}

		if (SUCCEEDED(hr)) {
			hr = buffer->Unlock();
			bufferData = nullptr;
		}

		if (SUCCEEDED(hr)) {
			hr = buffer->SetCurrentLength(bufferSize);
		}

		if (SUCCEEDED(hr)) {
			hr = MFCreateSample(&pSample);
		}

		if (SUCCEEDED(hr)) {
			hr = pSample->AddBuffer(buffer);
		}

		if (SUCCEEDED(hr)) {
			hr = pSample->SetSampleTime(currentPosition);
		}

		if (SUCCEEDED(hr)) {
			duration = 10 * 1000;
			hr = pSample->SetSampleDuration(duration);
		}

		if (SUCCEEDED(hr)) {
			if (discontinuity) {
				hr = pSample->SetUINT32(MFSampleExtension_Discontinuity, TRUE);
			}
		}

		if (SUCCEEDED(hr)) {
			currentPosition += duration;

			*sample = pSample;
			(*sample)->AddRef();
		}

		if (bufferData && buffer) {
			hr = buffer->Unlock();
		}

		return hr;
	}
	
	HRESULT PcmMediaStream::DeliverSample(IMFSample * sample)
	{
		HRESULT hr = S_OK;

		hr = mmf::QueueEventWithIUnknown(this, MEMediaSample, hr, sample);

		if (SUCCEEDED(hr)) {
			hr = CheckEndOfStream();
		}

		return hr;
	}
	
	HRESULT PcmMediaStream::DeliverQueuedSamples()
	{
		jni::ComPtr<IMFSample> pSample;
		HRESULT hr = S_OK;

		EnterCriticalSection(&criticalSection);

		if (eos) {
			hr = QueueEvent(MEEndOfStream, GUID_NULL, S_OK, NULL);
		}

		if (SUCCEEDED(hr)) {
			while (!sampleQueue.IsEmpty()) {
				hr = sampleQueue.Dequeue(&pSample);

				if (FAILED(hr)) {
					break;
				}

				hr = DeliverSample(pSample);

				if (FAILED(hr)) {
					break;
				}
			}
		}

		LeaveCriticalSection(&criticalSection);

		if (SUCCEEDED(hr)) {
			if (eos) {
				hr = mediaSource->QueueEvent(MEEndOfPresentation, GUID_NULL, S_OK, NULL);
			}
		}

		return hr;
	}

	void PcmMediaStream::Flush()
	{
		EnterCriticalSection(&criticalSection);

		sampleQueue.Clear();

		LeaveCriticalSection(&criticalSection);
	}

	LONGLONG PcmMediaStream::GetCurrentPosition() const
	{
		return currentPosition;
	}

	HRESULT PcmMediaStream::SetPosition(LONGLONG newPosition)
	{
		EnterCriticalSection(&criticalSection);
		/*
		// Check if the requested position is beyond the end of the stream.
		LONGLONG duration = AudioDurationFromBufferSize(m_pRiff->Format(), m_pRiff->Chunk().DataSize());

		if (newPosition > duration) {
			// Start position is past the end of the presentation.
			LeaveCriticalSection(&criticalSection);

			return MF_E_INVALIDREQUEST;
		}
		*/
		HRESULT hr = S_OK;
		/*
		if (currentPosition != newPosition) {
			LONGLONG offset = BufferSizeFromAudioDuration(m_pRiff->Format(), newPosition);

			hr = m_pRiff->MoveToChunkOffset((DWORD)offset);

			if (SUCCEEDED(hr)) {
				currentPosition = newPosition;
				discontinuity = TRUE;
				eos = FALSE;
			}
		}
		*/
		LeaveCriticalSection(&criticalSection);
		return hr;
	}

	HRESULT PcmMediaStream::CheckEndOfStream()
	{
		HRESULT hr = S_OK;
		/*
		if (m_pRiff->BytesRemainingInChunk() < m_pRiff->Format()->nBlockAlign) {
			eos = TRUE;

			hr = QueueEvent(MEEndOfStream, GUID_NULL, S_OK, NULL);
		}
		*/
		return hr;
	}
}