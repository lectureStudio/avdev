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

#include "WasapiAudioOutputStream.h"
#include "Log.h"
#include <cmath>

namespace avdev {

	WasapiAudioOutputStream::WasapiAudioOutputStream(std::wstring deviceId, PAudioSink sink) :
		WasapiAudioStream(deviceId),
		AudioOutputStream(sink),
		captureClient()
	{
	}

	float WasapiAudioOutputStream::getVolume()
	{
		return WasapiAudioStream::getVolume();
	}

	void WasapiAudioOutputStream::setVolume(float volume)
	{
		AudioOutputStream::setVolume(volume);
		WasapiAudioStream::setVolume(volume);
	}

	bool WasapiAudioOutputStream::getMute()
	{
		return WasapiAudioStream::getMute();
	}

	void WasapiAudioOutputStream::setMute(bool mute)
	{
		AudioOutputStream::setMute(mute);
		WasapiAudioStream::setMute(mute);
	}

	void WasapiAudioOutputStream::volumeChanged(float volume, bool mute)
	{
		//MessageQueue & mq = MessageQueue::instance();
		//mq.dispatch([this, volume, mute]() { listener->volumeChanged(volume, mute); });

		notifyVolumeChange(volume, mute);
	}

	void WasapiAudioOutputStream::openInternal()
	{
		float volume = AudioOutputStream::getVolume();
		unsigned latency = AudioOutputStream::getBufferLatency();
		AudioFormat format = AudioOutputStream::getAudioFormat();

		DWORD flags = AUDCLNT_STREAMFLAGS_NOPERSIST;

		WasapiAudioStream::initAudioClient(eCapture, flags, format, latency, this);
		WasapiAudioStream::setVolume(volume);

		HRESULT hr = audioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&captureClient);
		THROW_IF_FAILED(hr, "WASAPI: Get service failed.");

		// Calculate the buffer size for the specified stream latency.
		int ioSize = (format.getSampleRate() * format.getChannels() * (format.bitsPerSample() / 8) * latency) / 1000;

		initAudioBuffer(ioSize);
	}

	void WasapiAudioOutputStream::closeInternal()
	{
		WasapiAudioStream::shutdown();

		freeAudioBuffer();
	}

	void WasapiAudioOutputStream::startInternal()
	{
		WasapiAudioStream::start();
	}

	void WasapiAudioOutputStream::stopInternal()
	{
		WasapiAudioStream::stop();
	}

	void WasapiAudioOutputStream::processAudio()
	{
		HANDLE waitArray[2] = { threadShutdownEvent, samplesReadyEvent };
		WAVEFORMATEX * format = nullptr;
		UINT32 bufferFrames;
		UINT32 packetLength = 0;
		WORD frameSize;
		REFERENCE_TIME defPeriod;
		REFERENCE_TIME minPeriod;
		HRESULT hr;

		hr = audioClient->GetBufferSize(&bufferFrames);
		THROW_IF_FAILED(hr, "WASAPI: Get buffer size failed.");

		hr = audioClient->GetMixFormat(&format);
		THROW_IF_FAILED(hr, "WASAPI: Get audio format failed.");

		hr = audioClient->GetDevicePeriod(&defPeriod, &minPeriod);
		THROW_IF_FAILED(hr, "WASAPI: Get device period failed.");



		frameSize = format->nBlockAlign;
		CoTaskMemFree(format);
		format = nullptr;
		
		initBuffer(bufferFrames * frameSize);

		BYTE * tempBuffer = buffer.data();
		UINT32 bufferSize = static_cast<UINT32>(buffer.size());

		// Capture loop.
		bool running = true;

		while (running) {
			DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
			switch (waitResult) {
				case WAIT_OBJECT_0 + 0:		// Thread Shutdown Event
					running = false;
					break;

				case WAIT_OBJECT_0 + 1:		// Samples Ready Event
				{
					BYTE * data = nullptr;
					UINT32 framesAvailable;
					UINT32 srcLength;
					DWORD flags;

					hr = captureClient->GetNextPacketSize(&packetLength);
					
					if (FAILED(hr)) {
						running = false;
						break;
					}

					while (packetLength != 0) {
						hr = captureClient->GetBuffer(&data, &framesAvailable, &flags, nullptr, nullptr);

						switch (HRESULT_CODE(hr)) {
							case S_OK:
								srcLength = framesAvailable * frameSize;

								running = processSamples(data, &tempBuffer, srcLength, bufferSize, flags);

								captureClient->ReleaseBuffer(framesAvailable);
								break;

							case AUDCLNT_E_DEVICE_INVALIDATED:			// The audio endpoint device has been unplugged, reconfigured, disabled or removed.
							case AUDCLNT_E_BUFFER_OPERATION_PENDING:	// Stream reset is in progress.
							case AUDCLNT_E_SERVICE_NOT_RUNNING:			// The Windows audio service is not running.
								running = false;
								break;
						}

						hr = captureClient->GetNextPacketSize(&packetLength);

						if (FAILED(hr)) {
							running = false;
							break;
						}
					}
					break;
				}
			}
		}
	}

	bool WasapiAudioOutputStream::processSamples(BYTE * src, BYTE ** dst, UINT32 srcLength, UINT32 dstLength, DWORD flags)
	{
		bool success = true;

		if (srcLength == 0 || src == nullptr) {
			// Ignore this process step. Can happen, if the input buffer is empty.
			return success;
		}

		if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
			// Fill the output buffer with 0s.
			memset(*dst, 0, min(srcLength, dstLength));
		}
		else {
			memcpy_s(*dst, dstLength, src, srcLength);
		}

		// Existing resampler indicates we have to convert the samples.
		if (resampler != nullptr) {
			HRESULT hr = resampler->ProcessInput(*dst, srcLength);
			if (SUCCEEDED(hr)) {
				DWORD bytesWritten;

				hr = resampler->ProcessOutput(dst, dstLength, &bytesWritten);
				if (SUCCEEDED(hr)) {
					writeAudio(*dst, bytesWritten);
				}
				else {
					LOGDEV_ERROR("WASAPI: Audio-Resampler process output failed: 0x%08lX.", hr);
					success = false;
				}
			}
			else {
				LOGDEV_ERROR("WASAPI: Audio-Resampler process input failed: 0x%08lX.", hr);
				success = false;
			}
		}
		else {
			writeAudio(*dst, srcLength);
		}

		return success;
	}

}