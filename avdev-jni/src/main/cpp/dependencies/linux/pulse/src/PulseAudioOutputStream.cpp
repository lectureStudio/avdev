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

#include "AVdevException.h"
#include "PulseAudioOutputStream.h"
#include "Log.h"

namespace avdev
{
	PulseAudioOutputStream::PulseAudioOutputStream(std::string name, PAudioSink sink) :
		PulseAudioStream(name),
		AudioOutputStream(sink)
	{
	}

	float PulseAudioOutputStream::getVolume()
	{
		return AudioOutputStream::getVolume();
		/*
		if (getState() == StreamState::CLOSED) {
			return AudioOutputStream::getVolume();
		}

		uint32_t index = pa_stream_get_index(stream);

		pa_operation * operation = pa_context_get_source_info_by_index(context, index, contextVolumeCallback, this);
		completeOperation(mainloop, operation);

		return PulseAudioStream::getVolume();
		*/
	}

	void PulseAudioOutputStream::setVolume(float volume)
	{
		AudioOutputStream::setVolume(volume);

		if (getState() == StreamState::CLOSED) {
			return;
		}

		pa_threaded_mainloop_lock(mainloop);
		uint32_t index = pa_stream_get_index(stream);
		pa_volume_t volNorm = static_cast<pa_volume_t>(volume * PA_VOLUME_NORM);
		pa_cvolume pa_volume;
		pa_cvolume_set(&pa_volume, getAudioFormat().getChannels(), volNorm);
		pa_operation * operation = pa_context_set_source_output_volume(context, index, &pa_volume, nullptr, nullptr);
		pa_operation_unref(operation);
		pa_threaded_mainloop_unlock(mainloop);
	}

	bool PulseAudioOutputStream::getMute()
	{
		return PulseAudioStream::getMute();
	}

	void PulseAudioOutputStream::setMute(bool mute)
	{
		AudioOutputStream::setMute(mute);

		if (getState() == StreamState::CLOSED) {
			return;
		}

		pa_threaded_mainloop_lock(mainloop);
		uint32_t index = pa_stream_get_index(stream);
		pa_operation * operation = pa_context_set_source_output_mute(context, index, (int) mute, nullptr, nullptr);
		pa_operation_unref(operation);
		pa_threaded_mainloop_unlock(mainloop);
	}

	void PulseAudioOutputStream::openInternal()
	{
		unsigned latency = AudioOutputStream::getBufferLatency();
		AudioFormat format = AudioOutputStream::getAudioFormat();

		pa_sample_spec spec = audioFormatToSampleSpec(format);

		if (!pa_sample_spec_valid(&spec)) {
			throw AVdevException("PulseAudio: Invalid sample spec.");
		}

		createContext("PulseInput");

		pa_channel_map channel_map;
		pa_channel_map_init_extend(&channel_map, spec.channels, PA_CHANNEL_MAP_DEFAULT);

		if (!pa_channel_map_compatible(&channel_map, &spec)) {
			throw AVdevException("PulseAudio: Channel map doesn't match sample specification.");
		}

		stream = pa_stream_new(context, "RecordStream", &spec, &channel_map);
		if (!stream) {
			throw AVdevException("PulseAudio: Failed to create audio capture stream.");
		}

		pa_stream_set_state_callback(stream, PulseAudioStream::streamStateCallback, this);
		pa_stream_set_read_callback(stream, streamReadCallback, this);

		uint32_t bufferSize = pa_usec_to_bytes(latency * 1000, &spec);

		pa_buffer_attr buffer_attributes;
		buffer_attributes.maxlength = static_cast<uint32_t>(-1);
		buffer_attributes.tlength = static_cast<uint32_t>(-1);
		buffer_attributes.minreq = static_cast<uint32_t>(-1);
		buffer_attributes.prebuf = static_cast<uint32_t>(-1);
		buffer_attributes.fragsize = bufferSize;

		int flags = PA_STREAM_START_CORKED |
			PA_STREAM_INTERPOLATE_TIMING |
			PA_STREAM_AUTO_TIMING_UPDATE |
			PA_STREAM_NOT_MONOTONIC |
			PA_STREAM_ADJUST_LATENCY;

		pa_stream_flags_t flags_t = static_cast<pa_stream_flags_t>(flags);

		if (pa_stream_connect_record(stream, name.c_str(), &buffer_attributes, flags_t) < 0) {
			throw AVdevException("PulseAudio: Failed to connect to audio capture stream.");
		}

		if (!isStreamReady(stream, mainloop)) {
			throw AVdevException("PulseAudio: Failed to wait for capture stream.");
		}

		pa_threaded_mainloop_unlock(mainloop);

		setMute(AudioOutputStream::getMute());
		setVolume(AudioOutputStream::getVolume());

		initBuffer(bufferSize * 2);

		// Calculate the buffer size for the specified stream latency.
		int ioSize = (format.getSampleRate() * format.getChannels() * (format.bitsPerSample() / 8) * latency) / 1000;

		initAudioBuffer(ioSize);
	}

	void PulseAudioOutputStream::closeInternal()
	{
		PulseAudioStream::close();

		freeAudioBuffer();
	}

	void PulseAudioOutputStream::startInternal()
	{
		if (mainloop == nullptr) {
			return;
		}

		pa_threaded_mainloop_lock(mainloop);
		// Drop old data.
		pa_stream_drop(stream);
		pauseStream(false);
		pa_threaded_mainloop_unlock(mainloop);
	}

	void PulseAudioOutputStream::stopInternal()
	{
		PulseAudioStream::stop();
	}

	void PulseAudioOutputStream::streamReadCallback(pa_stream * paStream, size_t length, void * userdata)
	{
		if (length < 1) {
			throw AVdevException("PulseAudio: Stream read zero.");
		}

		const void * data;

		if (pa_stream_peek(paStream, &data, &length) < 0) {
			throw AVdevException("PulseAudio: Stream peek failed.");
		}

		const std::uint8_t * samples = static_cast<const std::uint8_t *>(data);

		PulseAudioOutputStream * stream = reinterpret_cast<PulseAudioOutputStream *>(userdata);
		stream->writeAudio(samples, length);

		pa_stream_drop(paStream);
		pa_threaded_mainloop_signal(stream->mainloop, 0);
	}
}
