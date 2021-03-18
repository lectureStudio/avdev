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

#include "MFAudioResampler.h"
#include "AVdevException.h"
#include "Log.h"

namespace avdev {

	MFAudioResampler::MFAudioResampler() :
		initialized(false),
		resampler(),
		inputBuffer(),
		outputBuffer(),
		inputSamples(),
		outputSamples()
	{
	}

	MFAudioResampler::~MFAudioResampler()
	{
	}

	void MFAudioResampler::Init(const WAVEFORMATEX * fmtIn, const WAVEFORMATEX * fmtOut, DWORD inputBufferSize, DWORD outputBufferSize, LONG filterQuality)
	{
		if (initialized)
			throw AVdevException("MFAudioResampler: Already initialized. Call Dispose() first.");

		IWMResamplerProps * resamplerProps = nullptr;
		jni::ComPtr<IMFMediaType> inputType;
		jni::ComPtr<IMFMediaType> outputType;
		HRESULT hr;

		hr = CoCreateInstance(CLSID_CResamplerMediaObject, nullptr, CLSCTX_INPROC_SERVER, IID_IMFTransform, (void**)&resampler);
		THROW_IF_FAILED(hr, "MFAudioResampler: Create resampler media object failed.");

		hr = resampler->QueryInterface(__uuidof(IWMResamplerProps), (void**)&resamplerProps);
		THROW_IF_FAILED(hr, "MFAudioResampler: Get properties failed.");

		hr = CreateMediaType(fmtIn, &inputType);
		THROW_IF_FAILED(hr, "MFAudioResampler: Create input media type failed.");

		hr = CreateMediaType(fmtOut, &outputType);
		THROW_IF_FAILED(hr, "MFAudioResampler: Create output media type failed.");

		hr = resampler->SetInputType(0, inputType, 0);
		THROW_IF_FAILED(hr, "MFAudioResampler: Set audio resampler input type failed.");

		hr = resampler->SetOutputType(0, outputType, 0);
		THROW_IF_FAILED(hr, "MFAudioResampler: Set audio resampler output type failed.");

		resamplerProps->SetHalfFilterLength(filterQuality);

		CreateMediaBuffer(inputBufferSize, &inputBuffer, &inputSamples);
		CreateMediaBuffer(outputBufferSize, &outputBuffer, &outputSamples);

		initialized = true;
	}

	/// <summary>
	/// Take audio data captured from WASAPI and feed it as input to audio resampler.
	/// </summary>
	/// <param name="pBuffer">
	/// [in] Buffer holding audio data from WASAPI.
	/// </param>
	/// <param name="bufferSize">
	/// [in] Number of bytes available in pBuffer.
	/// </param>
	/// <param name="flags">
	/// [in] Flags returned from WASAPI capture.
	/// </param>
	/// <returns>
	/// S_OK on success, otherwise failure code.
	/// </returns>
	HRESULT MFAudioResampler::ProcessInput(BYTE * sampleBuffer, DWORD bufferSize)
	{
		if (!initialized)
			throw AVdevException("MFAudioResampler: Not initialized.");

		BYTE * buffer = NULL;
		DWORD maxLength;
		HRESULT hr;

		hr = inputBuffer->Lock(&buffer, &maxLength, NULL);

		if (SUCCEEDED(hr)) {
			DWORD dataToCopy = min(bufferSize, maxLength);

			memcpy_s(buffer, maxLength, sampleBuffer, dataToCopy);

			hr = inputBuffer->SetCurrentLength(dataToCopy);

			if (SUCCEEDED(hr)) {
				hr = resampler->ProcessInput(0, inputSamples, 0);
			}

			inputBuffer->Unlock();
		}
		else {
			LOGDEV_ERROR("MFAudioResampler: Lock Failed: hr = 0x%08x.", hr);
		}

		return hr;
	}

	/// <summary>
	/// Get data output from audio resampler and write it to file.
	/// </summary>
	/// <param name="pBytesWritten">
	/// [out] On success, will receive number of bytes written to file.
	/// </param>
	/// <returns>
	/// S_OK on success, otherwise failure code.
	/// </returns>
	HRESULT MFAudioResampler::ProcessOutput(BYTE ** sampleBuffer, DWORD bufferSize, DWORD * bytesWritten)
	{
		if (!initialized)
			throw AVdevException("MFAudioResampler: Not initialized.");

		DWORD outStatus;
		HRESULT hr;

		MFT_OUTPUT_DATA_BUFFER outBuffer;
		outBuffer.pSample = outputSamples;
		outBuffer.dwStreamID = 0;
		outBuffer.dwStatus = 0;
		outBuffer.pEvents = 0;

		hr = resampler->ProcessOutput(0, 1, &outBuffer, &outStatus);

		if (SUCCEEDED(hr)) {
			BYTE * buffer = NULL;

			hr = outputBuffer->Lock(&buffer, NULL, NULL);

			if (SUCCEEDED(hr)) {
				DWORD lockedLength;
				hr = outputBuffer->GetCurrentLength(&lockedLength);

				if (SUCCEEDED(hr)) {
					memcpy_s(*sampleBuffer, bufferSize, buffer, lockedLength);

					*bytesWritten = lockedLength;
				}

				outputBuffer->Unlock();
			}
			else {
				LOGDEV_ERROR("MFAudioResampler: Lock Failed: hr = 0x%08x.", hr);
			}
		}

		return hr;
	}

	/// <summary>
	/// Gets the appropriate Media Foundation audio media subtype from the specified wave format.
	/// </summary>
	/// <param name="pwfx">
	/// [in] Input wave format to convert.
	/// </param>
	/// <param name="pSubType">
	/// [out] Media Foundation audio subtype resulting from conversion.
	/// </param>
	/// <returns>
	/// S_OK on success, otherwise failure code.
	/// </returns>
	HRESULT MFAudioResampler::GetMediaSubtype(const WAVEFORMATEX * fmt, GUID * subType)
	{
		HRESULT hr = S_OK;

		switch (fmt->wFormatTag) {
			case WAVE_FORMAT_PCM:
			case WAVE_FORMAT_IEEE_FLOAT:
			case WAVE_FORMAT_DTS:
			case WAVE_FORMAT_DOLBY_AC3_SPDIF:
			case WAVE_FORMAT_DRM:
			case WAVE_FORMAT_WMAUDIO2:
			case WAVE_FORMAT_WMAUDIO3:
			case WAVE_FORMAT_WMAUDIO_LOSSLESS:
			case WAVE_FORMAT_WMASPDIF:
			case WAVE_FORMAT_WMAVOICE9:
			case WAVE_FORMAT_MPEGLAYER3:
			case WAVE_FORMAT_MPEG:
			case WAVE_FORMAT_MPEG_HEAAC:
			case WAVE_FORMAT_MPEG_ADTS_AAC:
			{
				// Given an audio format tag, you can create an audio subtype GUID as follows
				//	1. Start with the value MFAudioFormat_Base, which is defined in "mfaph.i".
				//	2. Replace the first DWORD of this GUID with the format tag.

				GUID guidSubType = MFAudioFormat_Base;
				guidSubType.Data1 = fmt->wFormatTag;

				*subType = guidSubType;
				break;
			}
			case WAVE_FORMAT_EXTENSIBLE:
			{
				const WAVEFORMATEXTENSIBLE * extensible = reinterpret_cast<const WAVEFORMATEXTENSIBLE *>(fmt);

				if (extensible->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
					*subType = MFAudioFormat_PCM;
				}
				else if (extensible->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
					*subType = MFAudioFormat_Float;
				}
				else {
					hr = E_INVALIDARG;
				}
				break;
			}
			default:
			{
				hr = E_INVALIDARG;
				break;
			}
		}

		return hr;
	}

	HRESULT MFAudioResampler::CreateMediaType(const WAVEFORMATEX * fmt, IMFMediaType ** mediaType)
	{
		jni::ComPtr<IMFMediaType> type;
		GUID subType;
		HRESULT hr;

		hr = MFCreateMediaType(&type);
		if (FAILED(hr)) {
			return hr;
		}

		hr = GetMediaSubtype(fmt, &subType);
		if (FAILED(hr)) {
			return hr;
		}

		// Calculate derived values.
		UINT32 blockAlign = fmt->nChannels * (fmt->wBitsPerSample / 8);
		UINT32 bytesPerSecond = blockAlign * fmt->nSamplesPerSec;

		type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
		type->SetGUID(MF_MT_SUBTYPE, subType);
		type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, fmt->nChannels);
		type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, fmt->nSamplesPerSec);
		type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, blockAlign);
		type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bytesPerSecond);
		type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, fmt->wBitsPerSample);
		type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);

		*mediaType = type;
		(*mediaType)->AddRef();

		return hr;
	}

	/// <summary>
	/// Create a media buffer to be used as input or output for resampler.
	/// </summary>
	/// <param name="bufferSize">
	/// [in] Size of buffer to create.
	/// </param>
	/// <param name="ppSample">
	/// [out] Media Foundation sample created.
	/// </param>
	/// <param name="ppBuffer">
	/// [out] Media buffer created.
	/// </param>
	/// <returns>
	/// S_OK on success, otherwise failure code.
	/// </returns>
	void MFAudioResampler::CreateMediaBuffer(DWORD bufferSize, IMFMediaBuffer ** pBuffer, IMFSample ** pSamples)
	{
		jni::ComPtr<IMFSample> samples;
		jni::ComPtr<IMFMediaBuffer> buffer;
		HRESULT hr;

		hr = MFCreateSample(&samples);
		THROW_IF_FAILED(hr, "WASAPI: Create sample container failed.");

		hr = MFCreateMemoryBuffer(bufferSize, &buffer);
		THROW_IF_FAILED(hr, "WASAPI: Create media memory buffer failed.");

		hr = samples->AddBuffer(buffer);
		THROW_IF_FAILED(hr, "WASAPI: Add media buffer to sample container failed.");

		*pSamples = samples;
		(*pSamples)->AddRef();

		*pBuffer = buffer;
		(*pBuffer)->AddRef();
	}

}