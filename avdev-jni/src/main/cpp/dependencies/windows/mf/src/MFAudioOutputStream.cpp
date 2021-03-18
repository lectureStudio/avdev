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

#include "MFAudioOutputStream.h"
#include "MFTypeConverter.h"
#include "MFUtils.h"
#include "MFInitializer.h"
#include "WindowsHelper.h"

namespace avdev
{
	MFAudioOutputStream::MFAudioOutputStream(std::string endpointId, PAudioSink sink) :
		AudioOutputStream(sink),
		endpointId(endpointId)
	{
	}

	float MFAudioOutputStream::getVolume()
	{
		if (!streamVolume) {
			return AudioStream::getVolume();
		}

		float volume = 0;
		
		HRESULT hr = streamVolume->GetMasterVolume(&volume);
		THROW_IF_FAILED(hr, "MMF: Get audio capture master volume failed.");

		return volume;
	}

	void MFAudioOutputStream::setVolume(float volume)
	{
		AudioStream::setVolume(volume);

		if (streamVolume) {
			HRESULT hr = streamVolume->SetMasterVolume(volume);
			THROW_IF_FAILED(hr, "MMF: Set audio capture master volume failed.");
		}
	}

	bool MFAudioOutputStream::getMute()
	{
		if (!streamVolume) {
			return AudioStream::getMute();
		}

		BOOL mute = FALSE;

		HRESULT hr = streamVolume->GetMute(&mute);
		THROW_IF_FAILED(hr, "MMF: Get audio capture mute failed.");

		return (mute == TRUE);
	}

	void MFAudioOutputStream::setMute(bool mute)
	{
		AudioStream::setMute(mute);

		if (streamVolume) {
			HRESULT hr = streamVolume->SetMute(static_cast<BOOL>(mute));
			THROW_IF_FAILED(hr, "MMF: Set audio capture mute failed.");
		}
	}

	void MFAudioOutputStream::openInternal()
	{
		MFInitializer initializer;
		AudioFormat format = getAudioFormat();
		jni::ComPtr<IMFMediaType> sinkMediaType;

		// Create the output media type.
		GUID subType = MFTypeConverter::toApiType(format.getSampleFormat());
		mmf::CreateAudioMediaType(subType, format.getSampleRate(), format.getChannels(), format.bitsPerSample(), &sinkMediaType);

		MFOutputStream::createSession(endpointId, sinkMediaType);
		MFOutputStream::openSession();

		// Calculate the buffer size for the specified stream latency.
		unsigned latency = AudioOutputStream::getBufferLatency();
		int ioSize = (format.getSampleRate() * format.getChannels() * (format.bitsPerSample() / 8) * latency) / 1000;

		initAudioBuffer(ioSize);
	}

	void MFAudioOutputStream::closeInternal()
	{
		MFOutputStream::closeSession();
	}

	void MFAudioOutputStream::startInternal()
	{
		MFOutputStream::startSession();
	}

	void MFAudioOutputStream::stopInternal()
	{
		MFOutputStream::stopSession();
	}

	void MFAudioOutputStream::onTopologyReady()
	{
		MFInitializer initializer;
		HRESULT hr = MFGetService(session, MR_CAPTURE_POLICY_VOLUME_SERVICE, IID_PPV_ARGS(&streamVolume));
		THROW_IF_FAILED(hr, "MMF: Get audio capture volume service failed.");

		setMute(AudioStream::getMute());
		setVolume(AudioStream::getVolume());
	}

	void MFAudioOutputStream::processSample(const BYTE * sampleBuffer, DWORD & sampleSize)
	{
		writeAudio(sampleBuffer, sampleSize);
	}
}