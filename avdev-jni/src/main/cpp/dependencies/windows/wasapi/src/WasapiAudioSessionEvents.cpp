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

#include "WasapiAudioSessionEvents.h"

namespace avdev
{
	WasapiAudioSessionEvents::WasapiAudioSessionEvents(AudioSessionListener * listener) :
		refCount(1),
		listener(listener)
	{
	}

	// IUnknown methods -- AddRef, Release, and QueryInterface

	ULONG WasapiAudioSessionEvents::AddRef()
	{
		return InterlockedIncrement(&refCount);
	}

	ULONG WasapiAudioSessionEvents::Release()
	{
		ULONG ulRef = InterlockedDecrement(&refCount);
		if (0 == ulRef) {
			delete this;
		}
		return ulRef;
	}

	HRESULT WasapiAudioSessionEvents::QueryInterface(REFIID riid, VOID** ppvInterface)
	{
		if (IID_IUnknown == riid) {
			AddRef();
			*ppvInterface = (IUnknown*)this;
		}
		else if (__uuidof(IAudioSessionEvents) == riid) {
			AddRef();
			*ppvInterface = (IAudioSessionEvents*)this;
		}
		else {
			*ppvInterface = NULL;
			return E_NOINTERFACE;
		}
		return S_OK;
	}

	// Notification methods for audio session events

	HRESULT WasapiAudioSessionEvents::OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext)
	{
		listener->volumeChanged(NewVolume, NewMute);

		return S_OK;
	}
}