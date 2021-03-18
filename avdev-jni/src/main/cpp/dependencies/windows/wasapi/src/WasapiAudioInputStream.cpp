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

#include "WasapiAudioInputStream.h"
#include "Log.h"

namespace avdev {

	WasapiAudioInputStream::WasapiAudioInputStream(std::wstring deviceId, PAudioSource source) :
		WasapiAudioStream(deviceId),
		AudioInputStream(source),
		renderClient()
	{
	}

	float WasapiAudioInputStream::getVolume()
	{
		return WasapiAudioStream::getVolume();
	}

	void WasapiAudioInputStream::setVolume(float volume)
	{
		AudioInputStream::setVolume(volume);
		WasapiAudioStream::setVolume(volume);
	}

	bool WasapiAudioInputStream::getMute()
	{
		return WasapiAudioStream::getMute();
	}

	void WasapiAudioInputStream::setMute(bool mute)
	{
		AudioInputStream::setMute(mute);
		WasapiAudioStream::setMute(mute);
	}

	void WasapiAudioInputStream::volumeChanged(float volume, bool mute)
	{
		//MessageQueue & mq = MessageQueue::instance();
		//mq.dispatch([this, volume, mute]() { listener->volumeChanged(volume, mute); });

		notifyVolumeChange(volume, mute);
	}

	void WasapiAudioInputStream::openInternal()
	{
		float volume = AudioInputStream::getVolume();
		unsigned latency = AudioInputStream::getBufferLatency();
		AudioFormat format = AudioInputStream::getAudioFormat();

		DWORD flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST;
		HRESULT hr;

		WasapiAudioStream::initAudioClient(eRender, flags, format, latency, this);
		WasapiAudioStream::setVolume(volume);

		hr = audioClient->GetService(__uuidof(IAudioRenderClient), (void**)&renderClient);
		THROW_IF_FAILED(hr, "WASAPI: Get service failed.");

		initAudioBuffer(AudioInputStream::getPlaybackBufferSize());
	}

	void WasapiAudioInputStream::closeInternal()
	{
		WasapiAudioStream::shutdown();

		freeAudioBuffer();
	}

	void WasapiAudioInputStream::startInternal()
	{
		WasapiAudioStream::start();
	}

	void WasapiAudioInputStream::stopInternal()
	{
		WasapiAudioStream::stop();
	}

	void WasapiAudioInputStream::processAudio()
	{
		HANDLE waitArray[2] = { threadShutdownEvent, samplesReadyEvent };
		WAVEFORMATEX * format;
		UINT32 bufferFrames;
		DWORD samplesPerSec;
		WORD frameSize;
		DWORD avgBytesPerSec;
		bool running = true;
		HRESULT hr;

		hr = audioClient->GetBufferSize(&bufferFrames);
		THROW_IF_FAILED(hr, "WASAPI: Get buffer size failed.");

		hr = audioClient->GetMixFormat(&format);
		THROW_IF_FAILED(hr, "WASAPI: Get audio format failed.");

		samplesPerSec = format->nSamplesPerSec;
		frameSize = format->nBlockAlign;
		CoTaskMemFree(format);
		format = NULL;

		UINT32 outputBufferSize = bufferFrames * frameSize;
		AudioFormat inFormat = getAudioFormat();
		UINT32 outputBlockAlign = inFormat.getChannels() * (inFormat.bitsPerSample() / 8);
		avgBytesPerSec = inFormat.getSampleRate() * outputBlockAlign;

		initBuffer(bufferFrames * frameSize);

		BYTE * tempBuffer = ioBuffer.data();
		UINT32 bufferSize = static_cast<UINT32>(buffer.size());

		while (running) {
			DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);

			switch (waitResult) {
			case WAIT_OBJECT_0 + 0:		// Thread Shutdown Event
				running = false;
				break;

			case WAIT_OBJECT_0 + 1:		// Samples Ready Event
				BYTE * data = NULL;
				UINT32 framesRequested;
				UINT32 padding;

				hr = audioClient->GetCurrentPadding(&padding);

				framesRequested = bufferFrames - padding;

				hr = renderClient->GetBuffer(framesRequested, &data);

				switch (HRESULT_CODE(hr)) {
				case S_OK:
					if (framesRequested != 0 && data != nullptr) {
						double bufferLatency = 1.0 * framesRequested / samplesPerSec;
						DWORD framesBytes = static_cast<DWORD>(bufferLatency * avgBytesPerSec + 0.5);

						// Always read an integral part.
						DWORD residual = framesBytes % outputBlockAlign;
						if (residual != 0) {
							framesBytes = framesBytes - residual + outputBlockAlign;
						}

						int read = readAudio(framesBytes);
						if (read < 0) {
							running = false;
						}
						else if (read > 0) {
							// Existing resampler indicates we have to convert the samples.
							if (resampler != nullptr) {
								HRESULT hr = resampler->ProcessInput(tempBuffer, framesBytes);
								if (SUCCEEDED(hr)) {
									DWORD bytesWritten;

									hr = resampler->ProcessOutput(&tempBuffer, bufferSize, &bytesWritten);

									if (SUCCEEDED(hr)) {
										memcpy_s(data, outputBufferSize, tempBuffer, min(bytesWritten, frameSize * framesRequested));
									}
									else {
										LOGDEV_ERROR("WASAPI: Audio-Resampler process output failed: 0x%08lX.", hr);
										running = false;
									}
								}
								else {
									LOGDEV_ERROR("WASAPI: Audio-Resampler process input failed: 0x%08lX.", hr);
									running = false;
								}
							}
							else {
								size_t outputBytes = framesRequested * frameSize;
								memcpy_s(data, outputBufferSize, tempBuffer, outputBytes);
							}
						}
					}

					hr = renderClient->ReleaseBuffer(framesRequested, 0);

					break;

				case AUDCLNT_E_DEVICE_INVALIDATED:			// The audio endpoint device has been unplugged, reconfigured, disabled or removed.
				case AUDCLNT_E_BUFFER_OPERATION_PENDING:	// Stream reset is in progress.
				case AUDCLNT_E_SERVICE_NOT_RUNNING:			// The Windows audio service is not running.
					running = false;
					break;
				}
				break;
			}
		}
	}

}