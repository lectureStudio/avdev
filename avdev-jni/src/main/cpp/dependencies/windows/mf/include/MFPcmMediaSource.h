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

#ifndef AVDEV_MF_PCM_MEDIA_SOURCE_H_
#define AVDEV_MF_PCM_MEDIA_SOURCE_H_

#include "AudioInputStream.h"
#include "ComPtr.h"

#include <Mfidl.h>

namespace avdev
{
	//MIDL_INTERFACE("279a808d-aec7-40c8-9c6b-a6b492c78a66")
	class MFPcmMediaSource : public IMFMediaSource
	{
		friend class PcmMediaStream;

		public:
			static HRESULT CreateInstance(PAudioSource audioSource, IMFMediaType * mediaType, REFIID iid, void ** source);

			// IUnknown
			STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
			STDMETHODIMP_(ULONG) AddRef();
			STDMETHODIMP_(ULONG) Release();

			// IMFMediaEventGenerator
			STDMETHODIMP BeginGetEvent(IMFAsyncCallback * callback, IUnknown * punkState);
			STDMETHODIMP EndGetEvent(IMFAsyncResult * result, IMFMediaEvent ** mediaEvent);
			STDMETHODIMP GetEvent(DWORD flags, IMFMediaEvent ** mediaEvent);
			STDMETHODIMP QueueEvent(MediaEventType eventType, REFGUID extendedType, HRESULT status, const PROPVARIANT * value);

			// IMFMediaSource
			STDMETHODIMP CreatePresentationDescriptor(IMFPresentationDescriptor ** presentationDescriptor);
			STDMETHODIMP GetCharacteristics(DWORD* characteristics);
			STDMETHODIMP Pause();
			STDMETHODIMP Shutdown();
			STDMETHODIMP Start(IMFPresentationDescriptor * presentationDescriptor, const GUID * timeFormat, const PROPVARIANT * startPosition);
			STDMETHODIMP Stop();

		private:
			// Constructor is private - client should use static CreateInstance method.
			MFPcmMediaSource(PAudioSource source, IMFMediaType * mediaType, HRESULT & hr);
			virtual ~MFPcmMediaSource();

			void CreatePresentationDescriptor();
			void CreatePcmStream(IMFStreamDescriptor * sd);
			void QueueNewStreamEvent(IMFPresentationDescriptor * pd);
			void ValidatePresentationDescriptor(IMFPresentationDescriptor * pd);

			enum class State
			{
				STOPPED,
				PAUSED,
				STARTED
			};

			LONGLONG GetCurrentPosition() const;
			State GetState() const;
			HRESULT CheckShutdown() const;

			PAudioSource source;

			jni::ComPtr<IMFMediaType> mediaType;

			// Event generator helper.
			jni::ComPtr<IMFMediaEventQueue> eventQueue;

			// Default presentation.
			jni::ComPtr<IMFPresentationDescriptor> presentationDescriptor;

			// Media stream. Can be NULL, if no stream is selected.
			jni::ComPtr<PcmMediaStream> stream;

			// Reference count.
			long refCount;

			// Flag to indicate if Shutdown() method was called.
			BOOL shutdown;

			// Current state (running, stopped, paused).
			State state;

			CRITICAL_SECTION criticalSection;
	};

	class SampleQueue
	{
		public:
			SampleQueue()
			{
				anchorNode.next = &anchorNode;
				anchorNode.prev = &anchorNode;
			}

			virtual ~SampleQueue()
			{
				Clear();
			}

			HRESULT Queue(IMFSample * item)
			{
				if (!item) {
					return E_POINTER;
				}

				Node * node = new (std::nothrow) Node(item);

				if (node == nullptr) {
					return E_OUTOFMEMORY;
				}

				item->AddRef();

				Node * prevNode = anchorNode.prev;
				Node * nextNode = prevNode->next;

				prevNode->next = node;
				nextNode->prev = node;

				node->prev = prevNode;
				node->next = nextNode;

				return S_OK;
			}

			HRESULT Dequeue(IMFSample ** ppItem)
			{
				if (IsEmpty()) {
					return E_FAIL;
				}
				if (!ppItem) {
					return E_POINTER;
				}

				Node * node = anchorNode.next;

				// The next node's previous is this node's previous.
				node->next->prev = anchorNode.next->prev;

				// The previous node's next is this node's next.
				node->prev->next = node->next;

				*ppItem = node->item;
				delete node;

				return S_OK;
			}

			BOOL IsEmpty() const
			{
				return anchorNode.next == &anchorNode;
			}

			void Clear()
			{
				Node * node = anchorNode.next;

				// Delete the nodes
				while (node != &anchorNode) {
					if (node->item) {
						node->item->Release();
					}

					Node * tmp = node->next;
					delete node;
					node = tmp;
				}

				// Reset the anchor to point at itself.
				anchorNode.next = &anchorNode;
				anchorNode.prev = &anchorNode;
			}

		protected:
			// Nodes in the linked list.
			struct Node
			{
				Node * prev;
				Node * next;
				IMFSample * item;

				Node() : prev(nullptr), next(nullptr)
				{
				}

				Node(IMFSample * item) : prev(nullptr), next(nullptr)
				{
					this->item = item;
				}

				IMFSample * Item() const
				{
					return item;
				}
			};

		protected:
			Node anchorNode;
	};

	class PcmMediaStream : public IMFMediaStream
	{
		friend class MFPcmMediaSource;

		public:
			// IUnknown
			STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);
			STDMETHODIMP_(ULONG) AddRef();
			STDMETHODIMP_(ULONG) Release();

			// IMFMediaEventGenerator
			STDMETHODIMP BeginGetEvent(IMFAsyncCallback * callback, IUnknown * punkState);
			STDMETHODIMP EndGetEvent(IMFAsyncResult* result, IMFMediaEvent ** mediaEvent);
			STDMETHODIMP GetEvent(DWORD flags, IMFMediaEvent ** mediaEvent);
			STDMETHODIMP QueueEvent(MediaEventType eventType, REFGUID extendedType, HRESULT status, const PROPVARIANT * value);

			// IMFMediaStream
			STDMETHODIMP GetMediaSource(IMFMediaSource ** mediaSource);
			STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor ** streamDescriptor);
			STDMETHODIMP RequestSample(IUnknown * token);

		private:
			PcmMediaStream(PAudioSource audioSource, IMFMediaType * mediaType, MFPcmMediaSource * source, IMFStreamDescriptor * sd, HRESULT & hr);
			~PcmMediaStream();

			HRESULT CheckShutdown() const;
			HRESULT Shutdown();
			HRESULT CreateAudioSample(IMFSample ** sample);
			HRESULT DeliverSample(IMFSample * sample);
			HRESULT DeliverQueuedSamples();
			void Flush();

			LONGLONG GetCurrentPosition() const;
			HRESULT SetPosition(LONGLONG newPosition);
			HRESULT CheckEndOfStream();

			PAudioSource source;

			jni::ComPtr<IMFMediaType> mediaType;

			DWORD bufferSize;

			// Reference count.
			long refCount;
			
			// Flag to indicate if source's Shutdown() method was called.
			BOOL shutdown;

			// Current position in the stream, in 100-ns units.
			LONGLONG currentPosition;

			// Is the next sample a discontinuity?
			BOOL discontinuity;

			// Did we reach the end of the stream?
			BOOL eos;

			// Parent media source.
			jni::ComPtr<MFPcmMediaSource> mediaSource;

			// Stream descriptor for this stream.
			jni::ComPtr<IMFStreamDescriptor> streamDescriptor;

			// Event generator helper.
			jni::ComPtr<IMFMediaEventQueue> eventQueue;

			// Queue for samples while paused.
			SampleQueue sampleQueue;

			CRITICAL_SECTION criticalSection;
	};
}

#endif