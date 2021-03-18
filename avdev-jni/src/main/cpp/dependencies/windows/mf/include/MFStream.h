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

#ifndef AVDEV_MF_STREAM_H_
#define AVDEV_MF_STREAM_H_

#include "ComPtr.h"
#include "MFInitializer.h"

#include <string>
#include <Mfidl.h>

namespace avdev
{
	class MFStream : public IMFAsyncCallback
	{
		public:
			MFStream();
			virtual ~MFStream();

			// IUnknown methods
			STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);
			STDMETHODIMP_(ULONG) AddRef();
			STDMETHODIMP_(ULONG) Release();

			// IMFAsyncCallback methods
			STDMETHODIMP GetParameters(DWORD * flags, DWORD * queue);
			STDMETHODIMP Invoke(IMFAsyncResult * asyncResult);

		protected:
			void createSession(std::string endpointId, IMFMediaType * mediaType, WCHAR * mmcssClass);
			void openSession();
			void startSession();
			void stopSession();
			void closeSession();
			void runSession();

			virtual void onTopologyReady() = 0;

			jni::ComPtr<IMFMediaSession> session;
			jni::ComPtr<IMFTopology> topology;
			jni::ComPtr<IMFActivate> sinkActivate;
			jni::ComPtr<IMFMediaSource> mediaSource;

		private:
			enum class MmcssState
			{
				Registering,
				Registered,
				Unregistering,
				Unregistered
			};

			void registerWithMMCSS();
			void unregisterWithMMCSS();

			static DWORD WINAPI run(void * context);

			HANDLE threadHandle;
			CRITICAL_SECTION criticalSection;

			MmcssState mmcssState;

			MFInitializer initializer;
			jni::ComPtr<IMFWorkQueueServices> queueServices;
	};
}

#endif