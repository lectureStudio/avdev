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

#include <sstream>
#include "AlsaAudioManager.h"
#include "Log.h"

namespace avdev
{
	AlsaAudioManager & AlsaAudioManager::getInstance()
	{
		static AlsaAudioManager instance;
		return instance;
	}

	AlsaAudioManager::AlsaAudioManager()
	{
		getAudioCaptureDevices();
		getAudioPlaybackDevices();
	}

	AlsaAudioManager::~AlsaAudioManager()
	{
	}

	std::set<PAudioCaptureDevice> AlsaAudioManager::getAudioCaptureDevices() {
		if (captureDevices.empty()) {
			enumerateDevices(SND_PCM_STREAM_CAPTURE);
		}

		return captureDevices.devices();
	}

	std::set<PAudioPlaybackDevice> AlsaAudioManager::getAudioPlaybackDevices() {
		if (playbackDevices.empty()) {
			enumerateDevices(SND_PCM_STREAM_PLAYBACK);
		}

		return playbackDevices.devices();
	}

	void AlsaAudioManager::enumerateDevices(snd_pcm_stream_t stream)
	{
		snd_ctl_t * handle;
		snd_ctl_card_info_t * info;
		snd_pcm_info_t * pcminfo;
		snd_ctl_card_info_alloca(&info);
		snd_pcm_info_alloca(&pcminfo);

		int dev;
		int error;
		int card = -1;

		while (snd_card_next(&card) == 0 && card > -1) {
			char hwname[32];
			sprintf(hwname, "hw:%d", card);

			if ((error = snd_ctl_open(&handle, hwname, 0)) < 0) {
				LOGDEV_ERROR("ALSA: Control open failed (%i): %s.", card, snd_strerror(error));

				continue;
			}
			if ((error = snd_ctl_card_info(handle, info)) < 0) {
				LOGDEV_ERROR("ALSA: Get card related information failed (%i): %s.", card, snd_strerror(error));
				snd_ctl_close(handle);

				continue;
			}

			dev = -1;

			while (1) {
				if ((error = snd_ctl_pcm_next_device(handle, &dev)) < 0) {
					LOGDEV_ERROR("ALSA: Get next PCM device number failed (%i): %s.", card, snd_strerror(error));
				}
				if (dev < 0) {
					break;
				}

				snd_pcm_info_set_device(pcminfo, dev);
				snd_pcm_info_set_subdevice(pcminfo, 0);
				snd_pcm_info_set_stream(pcminfo, stream);

				if ((error = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
					if (error != -ENOENT) {
						LOGDEV_ERROR("ALSA: Get PCM device info failed (%i): %s.", card, snd_strerror(error));
					}
					continue;
				}

				std::ostringstream idStream;
				idStream << "plughw:" << card << "," << dev;

				std::ostringstream nameStream;
				nameStream << snd_ctl_card_info_get_name(info) << " [" << snd_pcm_info_get_name(pcminfo) << "]";

				std::string id = idStream.str();
				std::string name = nameStream.str();
				std::shared_ptr<AudioDevice> device = nullptr;

				if (stream == SND_PCM_STREAM_CAPTURE) {
					device = std::make_shared<AlsaAudioCaptureDevice>(name, id);
				}
				else if (stream == SND_PCM_STREAM_PLAYBACK) {
					device = std::make_shared<AlsaAudioPlaybackDevice>(name, id);
				}

				insertDevice(device, stream);
			}
			snd_ctl_close(handle);
		}
	}

	bool AlsaAudioManager::insertDevice(std::shared_ptr<AudioDevice> device, snd_pcm_stream_t stream)
	{
		if (device == nullptr) {
			return false;
		}

		if (stream == SND_PCM_STREAM_CAPTURE) {
			captureDevices.insertDevice(std::static_pointer_cast<AlsaAudioCaptureDevice>(device));
			return true;
		}
		else if (stream == SND_PCM_STREAM_PLAYBACK) {
			playbackDevices.insertDevice(std::static_pointer_cast<AlsaAudioPlaybackDevice>(device));
			return true;
		}

		return false;
	}
}
