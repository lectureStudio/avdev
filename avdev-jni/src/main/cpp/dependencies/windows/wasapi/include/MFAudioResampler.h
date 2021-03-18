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

#ifndef AVDEV_MF_AUDIO_RESAMPLER_H_
#define AVDEV_MF_AUDIO_RESAMPLER_H_

#include <Windows.h>
#include <Mfapi.h>
#include <Mftransform.h>
#include <Wmcodecdsp.h>
#include <Audioclient.h>
#include <atomic>

#include "windows/ComPtr.h"
#include "WindowsHelper.h"

namespace avdev {

	class MFAudioResampler
	{
		public:
			MFAudioResampler();
			~MFAudioResampler();

			/* Prevent copy and assignment. */
			MFAudioResampler(const MFAudioResampler &) = delete;
			MFAudioResampler & operator=(const MFAudioResampler &) = delete;

			void Init(const WAVEFORMATEX * fmtIn, const WAVEFORMATEX * fmtOut, DWORD inputBufferSize, DWORD outputBufferSize, LONG filterQuality);
			HRESULT ProcessInput(BYTE * sampleBuffer, DWORD bufferSize);
			HRESULT ProcessOutput(BYTE ** sampleBuffer, DWORD bufferSize, DWORD * bytesWritten);

		private:
			HRESULT GetMediaSubtype(const WAVEFORMATEX * fmt, GUID * subType);
			HRESULT CreateMediaType(const WAVEFORMATEX * fmt, IMFMediaType ** mediaType);
			void CreateMediaBuffer(DWORD bufferSize, IMFMediaBuffer ** pBuffer, IMFSample ** pSamples);

			std::atomic<bool> initialized;

			jni::ComPtr<IMFTransform> resampler;
			jni::ComPtr<IMFMediaBuffer> inputBuffer;
			jni::ComPtr<IMFMediaBuffer> outputBuffer;
			jni::ComPtr<IMFSample> inputSamples;
			jni::ComPtr<IMFSample> outputSamples;
	};

}

#endif