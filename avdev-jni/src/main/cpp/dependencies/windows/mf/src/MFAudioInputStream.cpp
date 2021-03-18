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

#include "MFAudioInputStream.h"
#include "MFTypeConverter.h"
#include "MFUtils.h"
#include "MFPcmMediaSource.h"
#include "MFInitializer.h"
#include "WindowsHelper.h"

namespace avdev
{
	MFAudioInputStream::MFAudioInputStream(std::string endpointId, PAudioSource source) :
		AudioInputStream(source),
		endpointId(endpointId)
	{
	}

	float MFAudioInputStream::getVolume()
	{
		float volume = AudioStream::getVolume();

		if (streamVolume) {
			UINT32 channels = getChannelCount();
			HRESULT hr;
			volume = 0;

			for (UINT32 i = 0; i < channels; i++) {
				float chVolume = 0;

				hr = streamVolume->GetChannelVolume(i, &chVolume);
				THROW_IF_FAILED(hr, "MMF: Get channel %d volume failed.", i);

				volume += chVolume;
			}

			volume /= channels;
		}

		return volume;
	}

	void MFAudioInputStream::setVolume(float volume)
	{
		AudioStream::setVolume(volume);

		if (streamVolume && !AudioStream::getMute()) {
			setVolumeAllChannels(volume);
		}
	}

	bool MFAudioInputStream::getMute()
	{
		bool mute = AudioStream::getMute();

		if (streamVolume) {
			UINT32 channels = getChannelCount();
			float volume = 0;
			HRESULT hr;

			for (UINT32 i = 0; i < channels; i++) {
				hr = streamVolume->GetChannelVolume(i, &volume);
				THROW_IF_FAILED(hr, "MMF: Get channel %d volume failed.", i);

				if (volume != 0) {
					return false;
				}
			}
		}

		return mute;
	}

	void MFAudioInputStream::setMute(bool mute)
	{
		AudioStream::setMute(mute);

		if (streamVolume) {
			if (mute) {
				setVolumeAllChannels(0);
			}
			else {
				setVolumeAllChannels(AudioStream::getVolume());
			}
		}
	}

	void MFAudioInputStream::openInternal()
	{
		MFInitializer initializer;
		AudioFormat format = getAudioFormat();
		jni::ComPtr<IMFMediaType> type;

		GUID subType = MFTypeConverter::toApiType(format.getSampleFormat());
		mmf::CreateAudioMediaType(subType, format.getSampleRate(), format.getChannels(), format.bitsPerSample(), &type);

		HRESULT hr = MFPcmMediaSource::CreateInstance(source, type, IID_PPV_ARGS(&mediaSource));
		THROW_IF_FAILED(hr, "MMF: Create PCM media source failed.");

		mmf::CreateSinkActivate(endpointId, &sinkActivate);

		MFStream::createSession(endpointId, type, L"Playback");
		MFStream::openSession();
	}

	void MFAudioInputStream::closeInternal()
	{
		MFStream::closeSession();
	}

	void MFAudioInputStream::startInternal()
	{
		MFStream::startSession();
	}

	void MFAudioInputStream::stopInternal()
	{
		MFStream::stopSession();
	}

	void MFAudioInputStream::onTopologyReady()
	{
		MFInitializer initializer;
		HRESULT hr = MFGetService(session, MR_STREAM_VOLUME_SERVICE, IID_PPV_ARGS(&streamVolume));
		THROW_IF_FAILED(hr, "MMF: Get audio stream volume service failed.");

		setMute(AudioStream::getMute());
		setVolume(AudioStream::getVolume());
	}

	UINT32 MFAudioInputStream::getChannelCount()
	{
		UINT32 channels = 0;

		if (streamVolume) {
			HRESULT hr = streamVolume->GetChannelCount(&channels);
			THROW_IF_FAILED(hr, "MMF: Get channel count failed.");
		}

		return channels;
	}
	
	void MFAudioInputStream::setVolumeAllChannels(float volume)
	{
		UINT32 channels = getChannelCount();
		HRESULT hr;

		for (UINT32 i = 0; i < channels; i++) {
			hr = streamVolume->SetChannelVolume(i, volume);
			THROW_IF_FAILED(hr, "MMF: Set channel %d volume failed.", i);
		}
	}
}