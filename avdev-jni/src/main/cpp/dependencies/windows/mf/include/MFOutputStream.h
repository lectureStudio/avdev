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

#ifndef AVDEV_MF_OUTPUT_STREAM_H_
#define AVDEV_MF_OUTPUT_STREAM_H_

#include "MFStream.h"
#include "ComPtr.h"

#include <string>
#include <Mfidl.h>

namespace avdev
{
	class MFOutputStream : public IMFSampleGrabberSinkCallback, protected MFStream
	{
		public:
			MFOutputStream();
			virtual ~MFOutputStream();

			// IUnknown methods
			STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);
			STDMETHODIMP_(ULONG) AddRef();
			STDMETHODIMP_(ULONG) Release();

			// IMFClockStateSink methods
			STDMETHODIMP OnClockStart(MFTIME systemTime, LONGLONG clockStartOffset);
			STDMETHODIMP OnClockStop(MFTIME systemTime);
			STDMETHODIMP OnClockPause(MFTIME systemTime);
			STDMETHODIMP OnClockRestart(MFTIME systemTime);
			STDMETHODIMP OnClockSetRate(MFTIME systemTime, float rate);

			// IMFSampleGrabberSinkCallback methods
			STDMETHODIMP OnSetPresentationClock(IMFPresentationClock * clock);
			STDMETHODIMP OnProcessSample(REFGUID majorMediaType, DWORD sampleFlags, LONGLONG sampleTime, LONGLONG sampleDuration, const BYTE * sampleBuffer, DWORD sampleSize);
			STDMETHODIMP OnShutdown();

		protected:
			void createSession(std::string deviceId, IMFMediaType * mediaType);

			virtual void processSample(const BYTE * sampleBuffer, DWORD & sampleSize) = 0;
	};
}

#endif