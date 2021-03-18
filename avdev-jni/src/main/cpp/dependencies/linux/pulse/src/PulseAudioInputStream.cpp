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
#include "PulseAudioInputStream.h"
#include "Log.h"

namespace avdev
{
	PulseAudioInputStream::PulseAudioInputStream(std::string name, PAudioSource source) :
		PulseAudioStream(name),
		AudioInputStream(source)
	{
	}

	float PulseAudioInputStream::getVolume()
	{
		return PulseAudioStream::getVolume();
	}

	void PulseAudioInputStream::setVolume(float volume)
	{
		AudioInputStream::setVolume(volume);

		if (getState() == StreamState::CLOSED) {
			return;
		}

		pa_threaded_mainloop_lock(mainloop);
		uint32_t index = pa_stream_get_index(stream);
		pa_volume_t volNorm = static_cast<pa_volume_t>(volume * PA_VOLUME_NORM);
		pa_cvolume pa_volume;
		pa_cvolume_set(&pa_volume, getAudioFormat().getChannels(), volNorm);
		pa_operation * operation = pa_context_set_sink_input_volume(context, index, &pa_volume, nullptr, nullptr);
		pa_operation_unref(operation);
		pa_threaded_mainloop_unlock(mainloop);
	}

	bool PulseAudioInputStream::getMute()
	{
		return PulseAudioStream::getMute();
	}

	void PulseAudioInputStream::setMute(bool mute)
	{
		AudioInputStream::setMute(mute);

		if (getState() == StreamState::CLOSED) {
			return;
		}

		pa_threaded_mainloop_lock(mainloop);
		uint32_t index = pa_stream_get_index(stream);
		pa_operation * operation = pa_context_set_sink_input_mute(context, index, (int) mute, nullptr, nullptr);
		pa_operation_unref(operation);
		pa_threaded_mainloop_unlock(mainloop);
	}

	void PulseAudioInputStream::openInternal()
	{
		unsigned latency = AudioInputStream::getBufferLatency();
		AudioFormat format = AudioInputStream::getAudioFormat();

		pa_sample_spec spec = audioFormatToSampleSpec(format);

		if (!pa_sample_spec_valid(&spec)) {
			throw AVdevException("PulseAudio: Invalid sample spec.");
		}

		createContext("PulseOutput");

		pa_channel_map channel_map;
		pa_channel_map_init_extend(&channel_map, spec.channels, PA_CHANNEL_MAP_DEFAULT);

		if (!pa_channel_map_compatible(&channel_map, &spec)) {
			throw AVdevException("PulseAudio: Channel map doesn't match sample specification.");
		}

		stream = pa_stream_new(context, "PlaybackStream", &spec, &channel_map);
		if (!stream) {
			throw AVdevException("PulseAudio: Failed to create audio playback stream.");
		}

		uint32_t bufferSize = pa_usec_to_bytes(latency * 1000, &spec);

		initAudioBuffer(bufferSize);

		pa_stream_set_state_callback(stream, PulseAudioStream::streamStateCallback, this);
		pa_stream_set_write_callback(stream, streamWriteCallback, this);

		pa_buffer_attr buffer_attributes;
		buffer_attributes.maxlength = static_cast<uint32_t>(-1);
		buffer_attributes.tlength = bufferSize;
		buffer_attributes.minreq = static_cast<uint32_t>(-1);
		buffer_attributes.prebuf = static_cast<uint32_t>(-1);
		buffer_attributes.fragsize = static_cast<uint32_t>(-1);

		int flags = PA_STREAM_START_CORKED |
			PA_STREAM_INTERPOLATE_TIMING |
			PA_STREAM_AUTO_TIMING_UPDATE |
			PA_STREAM_NOT_MONOTONIC |
			PA_STREAM_ADJUST_LATENCY;

		pa_stream_flags_t flags_t = static_cast<pa_stream_flags_t>(flags);

		if (pa_stream_connect_playback(stream, name.c_str(), &buffer_attributes, flags_t, nullptr, nullptr) < 0) {
			throw AVdevException("PulseAudio: Failed to connect playback stream.");
		}

		if (!isStreamReady(stream, mainloop)) {
			throw AVdevException("PulseAudio: Failed to wait for playback stream.");
		}

		pa_threaded_mainloop_unlock(mainloop);

		setMute(AudioInputStream::getMute());
		setVolume(AudioInputStream::getVolume());
	}

	void PulseAudioInputStream::closeInternal()
	{
		PulseAudioStream::close();

		freeAudioBuffer();
	}

	void PulseAudioInputStream::startInternal()
	{
		if (mainloop == nullptr) {
			return;
		}

		pa_threaded_mainloop_lock(mainloop);
		pauseStream(false);
		pa_threaded_mainloop_unlock(mainloop);
	}

	void PulseAudioInputStream::stopInternal()
	{
		PulseAudioStream::stop();
	}

	void PulseAudioInputStream::streamWriteCallback(pa_stream * paStream, size_t length, void * userdata)
	{
		if (length < 0) {
			throw AVdevException("PulseAudio: Stream write length is zero.");
		}

		PulseAudioInputStream * stream = reinterpret_cast<PulseAudioInputStream *>(userdata);

		int read = stream->readAudio(length);

		if (read < 0) {
			// Drain

			pa_threaded_mainloop_lock(stream->mainloop);

			pa_operation * op = pa_stream_drain(stream->stream, nullptr, nullptr);

			if (op != nullptr) {
				pa_operation_unref(op);
			}

			pa_threaded_mainloop_unlock(stream->mainloop);

			return;
		}

		if (pa_stream_write(paStream, stream->ioBuffer.data(), (size_t) read, 0, 0, PA_SEEK_RELATIVE) < 0) {
			throw AVdevException("PulseAudio: Failed to write to stream.");
		}
	}

}
