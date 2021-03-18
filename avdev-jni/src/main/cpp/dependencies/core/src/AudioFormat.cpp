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

#include "AudioFormat.h"
#include "AVdevException.h"

namespace avdev 
{
	AudioFormat::AudioFormat(SampleFormat format, unsigned sampleRate, unsigned channels)
	{
		this->sampleFormat = format;
		this->sampleRate = sampleRate;
		this->channels = channels;
	}

	bool AudioFormat::operator== (const AudioFormat & other) const
	{
		return (sampleFormat == other.sampleFormat &&
			sampleRate == other.sampleRate &&
			channels == other.channels);
	}

	bool AudioFormat::operator!= (const AudioFormat & other) const
	{
		return !(*this == other);
	}

	SampleFormat AudioFormat::getSampleFormat() const
	{
		return sampleFormat;
	}

	unsigned AudioFormat::getSampleRate() const
	{
		return sampleRate;
	}

	unsigned AudioFormat::getChannels() const
	{
		return channels;
	}

	unsigned AudioFormat::bitsPerSample() const
	{
		switch (sampleFormat) {
			case SampleFormat::ALAW:
			case SampleFormat::ULAW:
			case SampleFormat::U8:
				return 8;
			case SampleFormat::S16LE:
			case SampleFormat::S16BE:
				return 16;
			case SampleFormat::S24LE:
			case SampleFormat::S24BE:
				return 24;
			case SampleFormat::S32LE:
			case SampleFormat::S32BE:
			case SampleFormat::FLOAT32LE:
			case SampleFormat::FLOAT32BE:
				return 32;

			default:
				throw AVdevException("Could not retrieve Bits/Sample from sample format.");
		}
	}
}