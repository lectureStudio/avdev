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

#include <limits>
#include "AlsaAudioStream.h"
#include "Exception.h"
#include "Log.h"

namespace avdev
{
	AlsaAudioStream::AlsaAudioStream(std::string deviceId) :
		deviceId(deviceId),
		handle(nullptr),
		periodSize(0)
	{
	}

	AlsaAudioStream::~AlsaAudioStream()
	{
		close();
	}

	float AlsaAudioStream::getVolume()
	{
		float volume = 0;



		return volume;
	}

	void AlsaAudioStream::setVolume(float volume)
	{

	}

	bool AlsaAudioStream::getMute()
	{
		bool mute = true;



		return mute;
	}

	void AlsaAudioStream::setMute(bool mute)
	{

	}

	void AlsaAudioStream::open(snd_pcm_stream_t stream, AudioFormat & format, unsigned latency)
	{
		const char * devId = deviceId.c_str();
		int error;

		error = snd_pcm_open(&handle, devId, stream, 0);
		throwOnError(error, "ALSA: Open audio device failed: %s (%s).", devId);

		setHardwareParameters(format, latency);
		setSoftwareParameters(format, latency);

		error = snd_pcm_prepare(handle);
		throwOnError(error, "ALSA: Prepare audio interface failed: %s (%s).", devId);
	}

	void AlsaAudioStream::close()
	{
		if (handle != nullptr) {
			snd_pcm_close(handle);
		}

		handle = nullptr;
	}

	void AlsaAudioStream::start()
	{
		if (handle == nullptr) {
			return;
		}

		startThread();
	}

	void AlsaAudioStream::stop()
	{
		if (handle == nullptr) {
			return;
		}

		//int error;

		//if ((error = snd_pcm_pause(handle, 1)) < 0) {
		//	throw Exception("ALSA: Stop stream failed: %s.", snd_strerror(error));
		//}

		stopThreadAndWait();
	}

	void AlsaAudioStream::run()
	{
		if (handle == nullptr) {
			return;
		}

		// Run the loop.
		try {
			snd_pcm_sframes_t result;

			while (isRunning()) {
				processAudio(handle, &result);

				if (result == -EAGAIN) {
					snd_pcm_wait(handle, 100);
				}
				else if (result == -EPIPE || result == -ESTRPIPE) {
					if (!recovery(result)) {
						break;
					}
				}
				else if (result < 0) {
					break;
				}
			}
		}
		catch (Exception & ex) {
			LOGDEV_ERROR("ALSA: Process audio failed: %s.", ex.what());
		}
		catch (...) {
			try {
				std::rethrow_exception(std::current_exception());
			}
			catch (const std::exception & e) {
				LOGDEV_ERROR("ALSA: Unhandled exception caught: %s.", e.what());
			}
		}
	}

	void AlsaAudioStream::setHardwareParameters(AudioFormat & format, unsigned latency)
	{
		snd_pcm_hw_params_t * params;
		snd_pcm_format_t sampleFormat = SND_PCM_FORMAT_S16_LE;
		snd_pcm_uframes_t bufferSize;

		unsigned sampleRate = format.getSampleRate();
		unsigned channels = format.getChannels();
		unsigned periodTime = latency * 1000;	// ms to us
		unsigned bufferTime = periodTime * 4;

		const char * devId = deviceId.c_str();
		int error;

		error = snd_pcm_hw_params_malloc(&params);
		throwOnError(error, "ALSA: Allocate hardware parameters failed: %s.");

		error = snd_pcm_hw_params_any(handle, params);
		throwOnError(error, "ALSA: Initialize hardware parameters failed: %s (%s).", devId);

		error = snd_pcm_hw_params_set_rate_resample(handle, params, 1);
		throwOnError(error, "ALSA: Resampling setup failed: %s (%s).", devId);

		error = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
		throwOnError(error, "ALSA: Set parameters access failed: %s (%s).", devId);

		error = snd_pcm_hw_params_set_format(handle, params, sampleFormat);
		throwOnError(error, "ALSA: Set sample format failed: %s (%s).", devId);

		error = snd_pcm_hw_params_set_rate_near(handle, params, &sampleRate, 0);
		throwOnError(error, "ALSA: Set sample rate failed: %s (%s).", devId);

		error = snd_pcm_hw_params_set_channels(handle, params, channels);
		throwOnError(error, "ALSA: Set channels failed: %s (%s).", devId);

		if (sampleRate != format.getSampleRate()) {
			LOGDEV_WARN("ALSA: Rate doesn't match (requested %iHz, get %iHz).", format.getSampleRate(), sampleRate);
        }

		error = snd_pcm_hw_params_set_period_time_near(handle, params, &periodTime, 0);
		throwOnError(error, "ALSA: Set period time failed: %s (%s).", devId);

		error = snd_pcm_hw_params_set_buffer_time_near(handle, params, &bufferTime, 0);
		throwOnError(error, "ALSA: Set buffer time failed: %s (%s).", devId);

		error = snd_pcm_hw_params_get_period_size(params, &periodSize, 0);
		throwOnError(error, "ALSA: Get period size failed: %s (%s).", devId);

		error = snd_pcm_hw_params_get_buffer_size(params, &bufferSize);
		throwOnError(error, "ALSA: Get buffer size failed: %s (%s).", devId);

		if (periodSize == bufferSize) {
			throw Exception("ALSA: Period equal to buffer size: (%lu == %lu).", periodSize, bufferSize);
		}
		if ((error = snd_pcm_hw_params(handle, params)) < 0) {
			dump(DumpContext::PCM_HW_PARAMS, params);
			throwOnError(error, "ALSA: Set parameters failed: %s (%s).", devId);
		}

		dump(DumpContext::PCM_HW_PARAMS, params);

		snd_pcm_hw_params_free(params);
	}

	void AlsaAudioStream::setSoftwareParameters(AudioFormat & format, unsigned latency)
	{
		snd_pcm_sw_params_t * params;

		const char * devId = deviceId.c_str();

		int error;

		error = snd_pcm_sw_params_malloc(&params);
		throwOnError(error, "ALSA: Allocate software parameters failed: %s (%s).", devId);

		error = snd_pcm_sw_params_current(handle, params);
		throwOnError(error, "ALSA: Initialize software parameters failed: %s (%s).", devId);

		error = snd_pcm_sw_params_set_avail_min(handle, params, periodSize);
		throwOnError(error, "ALSA: Set minimum available frame count failed: %s (%s).", devId);

		error = snd_pcm_sw_params_set_start_threshold(handle, params, std::numeric_limits<int>::max());
		throwOnError(error, "ALSA: Set start threshold failed: %s (%s).", devId);

		if ((error = snd_pcm_sw_params(handle, params)) < 0) {
			dump(DumpContext::PCM_SW_PARAMS, params);
			throwOnError(error, "ALSA: Set software parameters failed: %s (%s).", devId);
		}

		dump(DumpContext::PCM_SW_PARAMS, params);

		snd_pcm_sw_params_free(params);
	}

	bool AlsaAudioStream::recovery(int error)
	{
		if (error == -EPIPE) {
			LOGDEV_WARN("ALSA: Xrun occurred.");

			if ((error = snd_pcm_prepare(handle)) < 0) {
				LOGDEV_ERROR("ALSA: Xrun prepare failed: %s.", snd_strerror(error));
				return false;
			}
		}
		else if (error == -ESTRPIPE) {
			LOGDEV_WARN("ALSA: Suspend occurred.");

			// Wait until the suspend flag is released.
			while ((error = snd_pcm_resume(handle)) == -EAGAIN) {
				sleep(1);
			}

			if (error < 0) {
				if ((error = snd_pcm_prepare(handle)) < 0) {
					LOGDEV_ERROR("ALSA: Suspend prepare failed: %s.", snd_strerror(error));
					return false;
				}
			}
		}
		return true;
	}

	void AlsaAudioStream::dump(DumpContext contextType, void * context)
	{
		snd_output_t * out;
		int error;

		error = snd_output_buffer_open(&out);
		throwOnError(error, "ALSA: Open log output buffer failed: %s.");

		switch (contextType) {
			case DumpContext::PCM_INFO:
				error = snd_pcm_dump(static_cast<snd_pcm_t *>(context), out);
				break;
			case DumpContext::PCM_HW_SETUP:
				error = snd_pcm_dump_hw_setup(static_cast<snd_pcm_t *>(context), out);
				break;
			case DumpContext::PCM_SW_SETUP:
				error = snd_pcm_dump_sw_setup(static_cast<snd_pcm_t *>(context), out);
				break;
			case DumpContext::PCM_SETUP:
				error = snd_pcm_dump_setup(static_cast<snd_pcm_t *>(context), out);
				break;
			case DumpContext::PCM_HW_PARAMS:
				error = snd_pcm_hw_params_dump(static_cast<snd_pcm_hw_params_t *>(context), out);
				break;
			case DumpContext::PCM_SW_PARAMS:
				error = snd_pcm_sw_params_dump(static_cast<snd_pcm_sw_params_t *>(context), out);
				break;
			case DumpContext::STATUS:
				error = snd_pcm_status_dump(static_cast<snd_pcm_status_t *>(context), out);
				break;
		}

		if (error < 0) {
			snd_output_close(out);
			throwOnError(error, "ALSA: Dump context failed: %s.");
		}

		char * str = nullptr;
        snd_output_buffer_string(out, &str);
        LOGDEV_INFO("ALSA DUMP:\n%s", str);

		snd_output_close(out);
	}

	template <typename ...Args>
	void AlsaAudioStream::throwOnError(int error, const char * message, Args && ...args)
	{
		if (error < 0) {
			throw Exception(message, std::forward<Args>(args)..., snd_strerror(error));
		}
	}
}
