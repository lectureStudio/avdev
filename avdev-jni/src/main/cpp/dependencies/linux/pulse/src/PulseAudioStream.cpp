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
#include "PulseAudioStream.h"
#include "Log.h"

namespace avdev
{
	PulseAudioStream::PulseAudioStream(std::string name) :
		name(name),
		mainloop(nullptr),
		context(nullptr),
		stream(nullptr)
	{
	}

	PulseAudioStream::~PulseAudioStream()
	{
		stop();
		close();
	}

	float PulseAudioStream::getVolume()
	{
		float volume = 0;

		

		return volume;
	}

	bool PulseAudioStream::getMute()
	{
		bool mute = true;

		

		return mute;
	}

	void PulseAudioStream::close()
	{
		dispose();
	}

	void PulseAudioStream::stop()
	{
		if (mainloop == nullptr) {
			return;
		}

		pa_threaded_mainloop_lock(mainloop);
		pauseStream(true);	
		pa_threaded_mainloop_unlock(mainloop);
	}
	
	void PulseAudioStream::dispose()
	{
		if (mainloop == nullptr) {
			return;
		}

		pa_threaded_mainloop_lock(mainloop);
		
		if (stream) {
			pa_stream_set_read_callback(stream, nullptr, nullptr);
			pa_stream_set_write_callback(stream, nullptr, nullptr);
			pa_stream_set_state_callback(stream, nullptr, nullptr);
			pa_stream_flush(stream, nullptr, nullptr);

			if (pa_stream_get_state(stream) != PA_STREAM_UNCONNECTED) {
				pa_stream_disconnect(stream);
			}

			pa_stream_unref(stream);
			stream = nullptr;
		}
		
		if (context) {
			pa_context_disconnect(context);
			pa_context_set_state_callback(context, nullptr, nullptr);
			pa_context_unref(context);
			context = nullptr;
		}
		
		pa_threaded_mainloop_unlock(mainloop);
		pa_threaded_mainloop_stop(mainloop);
		pa_threaded_mainloop_free(mainloop);
		mainloop = nullptr;
	}

	void PulseAudioStream::createContext(const char * name)
	{
		mainloop = pa_threaded_mainloop_new();
		
		if (!mainloop) {
			throw AVdevException("PulseAudio: Failed to create main loop.");
		}
		
		pa_mainloop_api * mainloopApi = pa_threaded_mainloop_get_api(mainloop);
		context = pa_context_new(mainloopApi, name);
		
		if (!context) {
			pa_threaded_mainloop_free(mainloop);
			
			throw AVdevException("PulseAudio: Failed to create context.");
		}
		
		pa_context_set_state_callback(context, contextStateCallback, mainloop);
		
		pa_threaded_mainloop_lock(mainloop);
		
		if (pa_threaded_mainloop_start(mainloop) != 0) {
			pa_context_unref(context);
			pa_threaded_mainloop_free(mainloop);
			
			throw AVdevException("PulseAudio: Failed start threaded mainloop.");
		}
		
		if (pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
			pa_context_unref(context);
			pa_threaded_mainloop_free(mainloop);
			
			throw AVdevException("PulseAudio: Failed to connect context.");
		}
		
		if (!isContextReady(context, mainloop)) {
			throw AVdevException("PulseAudio: Failed to wait for context.");
		}
	}
	
	void PulseAudioStream::contextStateCallback(pa_context * context, void * userdata)
	{
		pa_threaded_mainloop * mainloop = static_cast<pa_threaded_mainloop *>(userdata);
		pa_threaded_mainloop_signal(mainloop, 0);
	}

	void PulseAudioStream::contextVolumeCallback(pa_context * context, const pa_source_info * info, int error, void * userdata)
	{
		printf("contextVolumeCallback \n");
		fflush(NULL);

		//PulseAudioStream * stream = reinterpret_cast<PulseAudioStream *>(userdata);

		pa_volume_t volAvg = pa_cvolume_avg(&info->volume);

		float volume = volAvg / PA_VOLUME_NORM;


		printf("pa vol: %f \n", volume);
		fflush(NULL);
	}

	void PulseAudioStream::streamStateCallback(pa_stream * paStream, void * userdata)
	{
		PulseAudioStream * stream = reinterpret_cast<PulseAudioStream *>(userdata);

		if (paStream && pa_stream_get_state(paStream) == PA_STREAM_FAILED) {
			LOGDEV_ERROR("PulseAudio: Stream state error.");
		}

		pa_threaded_mainloop_signal(stream->mainloop, 0);
	}

	bool PulseAudioStream::isContextReady(pa_context * context, pa_threaded_mainloop * mainloop)
	{
		pa_context_state_t state;

		while ((state = pa_context_get_state(context)) != PA_CONTEXT_READY) {
			if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
				return false;
			}
			pa_threaded_mainloop_wait(mainloop);
		}
		return true;
	}

	bool PulseAudioStream::isStreamReady(pa_stream * paStream, pa_threaded_mainloop * mainloop)
	{
		pa_stream_state_t state;

		while ((state = pa_stream_get_state(paStream)) != PA_STREAM_READY) {
			if (state == PA_STREAM_FAILED || state == PA_STREAM_TERMINATED) {
				return false;
			}
			pa_threaded_mainloop_wait(mainloop);
		}
		return true;
	}
	
	void PulseAudioStream::completeOperation(pa_threaded_mainloop * mainloop, pa_operation * operation)
	{
		while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
			pa_threaded_mainloop_wait(mainloop);
		}
		
		pa_operation_unref(operation);
	}

	void PulseAudioStream::pauseStream(bool pause)
	{
		if (!stream || !mainloop) {
			throw AVdevException("PulseAudio: Pausing stream is not possible.");
		}

		pa_operation * operation = pa_stream_cork(stream, (int)pause, streamSuccessCallback, mainloop);
		completeOperation(mainloop, operation);
	}

	void PulseAudioStream::streamSuccessCallback(pa_stream * stream, int success, void * userdata)
	{
		pa_threaded_mainloop * pa_mainloop = static_cast<pa_threaded_mainloop *>(userdata);
		pa_threaded_mainloop_signal(pa_mainloop, 0);
	}
	
	pa_sample_spec PulseAudioStream::audioFormatToSampleSpec(const AudioFormat & format)
	{
		pa_sample_spec spec;
		spec.rate = format.getSampleRate();
		spec.channels = format.getChannels();
		
		switch (format.getSampleFormat()) {
			case SampleFormat::U8:
				spec.format = PA_SAMPLE_U8;
				break;
			case SampleFormat::S16LE:
				spec.format = PA_SAMPLE_S16LE;
				break;
			case SampleFormat::S16BE:
				spec.format = PA_SAMPLE_S16BE;
				break;
			case SampleFormat::S24LE:
				spec.format = PA_SAMPLE_S24LE;
				break;
			case SampleFormat::S24BE:
				spec.format = PA_SAMPLE_S24BE;
				break;
			case SampleFormat::S32LE:
				spec.format = PA_SAMPLE_S32LE;
				break;
			case SampleFormat::S32BE:
				spec.format = PA_SAMPLE_S32BE;
				break;
			case SampleFormat::FLOAT32LE:
				spec.format = PA_SAMPLE_FLOAT32LE;
				break;
			case SampleFormat::FLOAT32BE:
				spec.format = PA_SAMPLE_FLOAT32BE;
				break;
			case SampleFormat::ALAW:
				spec.format = PA_SAMPLE_ALAW;
				break;
			case SampleFormat::ULAW:
				spec.format = PA_SAMPLE_ULAW;
				break;
			default:
				spec.format = PA_SAMPLE_INVALID;
				break;
		}
		
		return spec;
	}
}
