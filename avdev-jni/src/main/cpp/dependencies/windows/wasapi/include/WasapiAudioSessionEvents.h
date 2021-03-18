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

#ifndef AVDEV_WASAPI_AUDIO_SESSION_EVENTS_H_
#define AVDEV_WASAPI_AUDIO_SESSION_EVENTS_H_

#include "AudioSessionListener.h"

#include <audiopolicy.h>

namespace avdev {

	class WasapiAudioSessionEvents : public IAudioSessionEvents
	{
		public:
			WasapiAudioSessionEvents(AudioSessionListener * listener);
			virtual ~WasapiAudioSessionEvents() {};

            HRESULT QueryInterface(REFIID riid, VOID** ppvInterface);
            ULONG AddRef();
            ULONG Release();

            HRESULT OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID EventContext) {
                return S_OK;
            }
            HRESULT OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID EventContext) {
                return S_OK;
            }
            HRESULT OnSimpleVolumeChanged(float NewVolume, BOOL NewMute,LPCGUID EventContext);
            HRESULT OnChannelVolumeChanged(DWORD ChannelCount, float NewChannelVolumeArray[], DWORD ChangedChannel, LPCGUID EventContext) {
                return S_OK;
            }
            HRESULT OnGroupingParamChanged(LPCGUID NewGroupingParam, LPCGUID EventContext) {
                return S_OK;
            }
            HRESULT OnStateChanged(AudioSessionState NewState) {
                return S_OK;
            }
            HRESULT OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason) {
                return S_OK;
            }
			
		private:
            LONG refCount;
            AudioSessionListener * listener;
	};
}

#endif