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

#include "WasapiAudioStream.h"
#include "WasapiAudioSessionEvents.h"
#include "ComInitializer.h"
#include "Log.h"

namespace avdev {

	WasapiAudioStream::WasapiAudioStream(std::wstring deviceId) :
		deviceId(deviceId),
		threadHandle(nullptr),
		threadShutdownEvent(nullptr),
		samplesReadyEvent(nullptr),
		device(),
		audioClient(),
		streamVolume(),
		sessionEvents(nullptr),
		resampler(nullptr),
		lastVolume(1.0f)
	{
	}

	WasapiAudioStream::~WasapiAudioStream()
	{
		if (resampler != nullptr) {
			delete resampler;
			resampler = nullptr;
		}
	}

	float WasapiAudioStream::getVolume()
	{
		float volume = 0;

		if (audioVolume != nullptr) {
			HRESULT hr = audioVolume->GetMasterVolume(&volume);
			THROW_IF_FAILED(hr, "WASAPI: Get volume failed.");
		}
		else if (streamVolume != nullptr) {
			UINT32 channels = GetChannelCount();

			float volumes[10];

			HRESULT hr = streamVolume->GetAllVolumes(channels, volumes);
			THROW_IF_FAILED(hr, "WASAPI: Get volumes failed.");

			for (UINT32 i = 0; i < channels; i++) {
				volume += volumes[i];
			}

			volume /= channels;
		}

		return volume;
	}

	void WasapiAudioStream::setVolume(float volume)
	{
		this->lastVolume = volume;

		if (audioVolume != nullptr) {
			HRESULT hr = audioVolume->SetMasterVolume(volume, &GUID_NULL);
			THROW_IF_FAILED(hr, "WASAPI: Set volume failed.");
		}
		else if (streamVolume != nullptr) {
			setVolumeAllChannels(volume);
		}
	}

	bool WasapiAudioStream::getMute()
	{
		BOOL mute = true;

		if (audioVolume != nullptr) {
			HRESULT hr = audioVolume->GetMute(&mute);
			THROW_IF_FAILED(hr, "WASAPI: Get mute failed.");
		}
		else if (streamVolume != nullptr) {
			UINT32 channels = GetChannelCount();

			float volumes[10];

			HRESULT hr = streamVolume->GetAllVolumes(channels, volumes);
			THROW_IF_FAILED(hr, "WASAPI: Get all volumes failed.");

			for (UINT32 i = 0; i < channels; i++) {
				if (volumes[i] != 0) {
					return false;
				}
			}
		}

		return mute;
	}

	void WasapiAudioStream::setMute(bool mute)
	{
		if (audioVolume != nullptr) {
			HRESULT hr = audioVolume->SetMute(mute, &GUID_NULL);
			THROW_IF_FAILED(hr, "WASAPI: Set mute failed.");
		}
		else if (streamVolume != nullptr) {
			if (mute) {
				setVolumeAllChannels(0);
			}
			else {
				setVolumeAllChannels(lastVolume);
			}
		}
	}

	float WasapiAudioStream::getBufferLatency()
	{
		if (audioClient == nullptr) {
			throw AVdevException("WASAPI: Audio client is not initialized.");
		}

		REFERENCE_TIME defPeriod;
		REFERENCE_TIME minPeriod;

		HRESULT hr = audioClient->GetDevicePeriod(&defPeriod, &minPeriod);
		THROW_IF_FAILED(hr, "WASAPI: Get device period failed.");

		return static_cast<float>(defPeriod / 10000.F);
	}

	UINT32 WasapiAudioStream::GetChannelCount()
	{
		UINT32 channels = 0;

		if (streamVolume != nullptr) {
			HRESULT hr = streamVolume->GetChannelCount(&channels);
			THROW_IF_FAILED(hr, "WASAPI: Get channel count failed.");
		}
		
		return channels;
	}

	void WasapiAudioStream::setVolumeAllChannels(float volume)
	{
		UINT32 channels = GetChannelCount();

		float volumes[10];
		for (UINT32 i = 0; i < channels; i++) {
			volumes[i] = volume;
		}

		HRESULT hr = streamVolume->SetAllVolumes(channels, volumes);
		THROW_IF_FAILED(hr, "WASAPI: Set all volumes failed.");
	}

	void WasapiAudioStream::initAudioClient(EDataFlow dataFlow, DWORD flags, AudioFormat format, unsigned latency, AudioSessionListener * listener)
	{
		jni::ComInitializer initializer;
		jni::ComPtr<IMMDeviceEnumerator> enumerator;
		WAVEFORMATEX * mixFormat = nullptr;
		HRESULT hr;

		sessionEvents = std::make_unique<WasapiAudioSessionEvents>(listener);

		threadShutdownEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
		THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()), "WASAPI: Create shutdown event failed.");

		samplesReadyEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
		THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()), "WASAPI: Create samples ready event failed.");

		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
		THROW_IF_FAILED(hr, "WASAPI: CoCreateInstance(IMMDeviceEnumerator) failed.");

		hr = enumerator->GetDevice(deviceId.c_str(), &device);
		THROW_IF_FAILED(hr, "WASAPI: Get device %S failed.", deviceId.c_str());

		hr = device->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, nullptr, reinterpret_cast<void**>(&audioClient));
		THROW_IF_FAILED(hr, "WASAPI: Activate device failed.");

		jni::ComPtr<IAudioSessionManager> manager;
		hr = device->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&manager));
		THROW_IF_FAILED(hr, "WASAPI: Get session manager failed.");

		hr = manager->GetAudioSessionControl(&GUID_NULL, 0, &sessionControl);
		THROW_IF_FAILED(hr, "WASAPI: Get session control failed.");

		hr = sessionControl->RegisterAudioSessionNotification(sessionEvents.get());
		THROW_IF_FAILED(hr, "WASAPI: Register for session notifications failed.");

		hr = audioClient->GetMixFormat(&mixFormat);
		THROW_IF_FAILED(hr, "WASAPI: Get audio format failed.");

		hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | flags, latency * 10000, 0, mixFormat, &GUID_NULL);
		THROW_IF_FAILED(hr, "WASAPI: Initialize audio client failed.");

		hr = audioClient->GetService(__uuidof(ISimpleAudioVolume), (void**)&audioVolume);
		THROW_IF_FAILED(hr, "WASAPI: Get audio volume failed.");

		hr = audioClient->GetService(__uuidof(IAudioStreamVolume), (void**)&streamVolume);
		THROW_IF_FAILED(hr, "WASAPI: Get stream volume failed.");

		hr = audioClient->SetEventHandle(samplesReadyEvent);
		THROW_IF_FAILED(hr, "WASAPI: Set event handle failed.");

		// Create resampler, if formats do not match.
		AudioFormat devAudioFormat(GetSampleFormat(mixFormat), mixFormat->nSamplesPerSec, mixFormat->nChannels);

		if (devAudioFormat != format) {
			UINT32 bufferFrames;
			hr = audioClient->GetBufferSize(&bufferFrames);
			THROW_IF_FAILED(hr, "WASAPI: Get buffer size failed.");

			WAVEFORMATEX userFormat = {};
			userFormat.wFormatTag = WAVE_FORMAT_PCM;
			userFormat.nChannels = format.getChannels();
			userFormat.nSamplesPerSec = format.getSampleRate();
			userFormat.wBitsPerSample = format.bitsPerSample();
			userFormat.nBlockAlign = userFormat.nChannels * (userFormat.wBitsPerSample / 8);
			userFormat.nAvgBytesPerSec = userFormat.nSamplesPerSec * userFormat.nBlockAlign;
			userFormat.cbSize = 0;

			// Calculate buffer latency with the current capture format.
			double bufferLatency = 1.0 * bufferFrames / mixFormat->nSamplesPerSec;
			DWORD deviceBufferSize = bufferFrames * mixFormat->nBlockAlign;
			DWORD userBufferSize = static_cast<DWORD>(bufferLatency * userFormat.nAvgBytesPerSec + 0.5);

			LOGDEV_INFO("Device Format: %d %d %d.", mixFormat->nChannels, mixFormat->wBitsPerSample, mixFormat->nSamplesPerSec);
			LOGDEV_INFO("User Format: %d %d %d.", format.getChannels(), format.bitsPerSample(), format.getSampleRate());
			LOGDEV_INFO("Buffer Latency: %f ms.", bufferLatency * 1000);
			LOGDEV_INFO("Device format buffer: %d bytes.", deviceBufferSize);
			LOGDEV_INFO("User format buffer: %d bytes.", userBufferSize);
			
			resampler = new MFAudioResampler();

			if (dataFlow == eCapture) {
				// Best quality.
				resampler->Init(mixFormat, &userFormat, deviceBufferSize, userBufferSize, 60);
			}
			else if (dataFlow == eRender) {
				// Best quality.
				resampler->Init(&userFormat, mixFormat, userBufferSize, deviceBufferSize, 60);
			}
		}

		CoTaskMemFree(mixFormat);
		mixFormat = nullptr;
	}

	void WasapiAudioStream::shutdown()
	{
		if (threadHandle) {
			SetEvent(threadShutdownEvent);
			WaitForSingleObject(threadHandle, INFINITE);
			CloseHandle(threadHandle);
			threadHandle = nullptr;
		}

		if (threadShutdownEvent != INVALID_HANDLE_VALUE) {
			CloseHandle(threadShutdownEvent);
			threadShutdownEvent = INVALID_HANDLE_VALUE;
		}

		if (samplesReadyEvent != INVALID_HANDLE_VALUE) {
			CloseHandle(samplesReadyEvent);
			samplesReadyEvent = INVALID_HANDLE_VALUE;
		}

		sessionControl->UnregisterAudioSessionNotification(sessionEvents.get());

		sessionEvents = nullptr;
		resampler = nullptr;
	}

	void WasapiAudioStream::start()
	{
		HRESULT hr;

		threadHandle = CreateThread(nullptr, 0, run, this, 0, nullptr);

		if (threadHandle == nullptr) {
			throw AVdevException("WASAPI: Create thread failed.");
		}

		hr = audioClient->Reset();
		THROW_IF_FAILED(hr, "WASAPI: Reset audio client failed.");

		hr = audioClient->Start();
		THROW_IF_FAILED(hr, "WASAPI: Start audio client failed.");
	}

	void WasapiAudioStream::stop()
	{
		// Tell the capture/playback thread to shut down.
		if (threadShutdownEvent) {
			SetEvent(threadShutdownEvent);
		}

		HRESULT hr = audioClient->Stop();
		THROW_IF_FAILED(hr, "WASAPI: Stop audio client failed.");

		if (threadHandle) {
			WaitForSingleObject(threadHandle, INFINITE);
			CloseHandle(threadHandle);
			threadHandle = nullptr;
		}
	}

	DWORD WINAPI WasapiAudioStream::run(LPVOID context)
	{
		WasapiAudioStream * stream = static_cast<WasapiAudioStream *>(context);

		jni::ComInitializer initializer;

		// Enable prioritized access to CPU resources.
		DWORD mmcssTaskIndex = 0;
		HANDLE mmcssHandle = AvSetMmThreadCharacteristicsW(L"Pro Audio", &mmcssTaskIndex);

		if (mmcssHandle) {
			AvSetMmThreadPriority(mmcssHandle, AVRT_PRIORITY_CRITICAL);
		}
		else {
			LOGDEV_WARN("WASAPI: Enable Multimedia Class Scheduler Service (MMCSS) failed.");
		}
		
		// Run the loop.
		try {
			stream->processAudio();
		}
		catch (AVdevException& ex) {
			LOGDEV_ERROR("WASAPI: Process audio failed: %s.", ex.what());
		}
		catch (...) {
			LOGDEV_ERROR("WASAPI: Unknown exception caught while processing audio.");
		}

		AvRevertMmThreadCharacteristics(mmcssHandle);

		return 0;
	}

}