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

package org.lecturestudio.avdev;

public class AudioFormat {

	public enum SampleFormat
	{
		/** Unsigned 8 Bit PCM */
		U8,
		/** Signed 16 Bit PCM, little endian */
		S16LE,
		/** Signed 16 Bit PCM, big endian */
		S16BE,
		/** Signed 24 Bit PCM, little endian */
		S24LE,
		/** Signed 24 Bit PCM, big endian */
		S24BE,
		/** Signed 32 Bit PCM, little endian */
		S32LE,
		/** Signed 32 Bit PCM, big endian */
		S32BE,
		/** 32 Bit IEEE floating point, little endian, range -1.0 to 1.0 */
		FLOAT32LE,
		/** 32 Bit IEEE floating point, big endian, range -1.0 to 1.0 */
		FLOAT32BE,
		/** 8 Bit a-Law */
		ALAW,
		/** 8 Bit mu-Law */
		ULAW
	}
	
	
	private final SampleFormat sampleFormat;
	private final int sampleRate;
	private final int channels;


	public AudioFormat(SampleFormat format, int sampleRate, int channels) {
		this.sampleFormat = format;
		this.sampleRate = sampleRate;
		this.channels = channels;
	}
	
	public SampleFormat getSampleFormat() {
		return sampleFormat;
	}

	public int getSampleRate() {
		return sampleRate;
	}

	public int getChannels() {
		return channels;
	}

	public int bitsPerSample() {
		switch (sampleFormat) {
			case ALAW:
			case ULAW:
			case U8:
				return 8;
			case S16LE:
			case S16BE:
				return 16;
			case S24LE:
			case S24BE:
				return 24;
			case S32LE:
			case S32BE:
			case FLOAT32LE:
			case FLOAT32BE:
				return 32;

			default:
				return 0;
		}
	}
	
	@Override
	public String toString() {
		return sampleFormat.toString() + ", " + sampleRate + ", " + channels;
	}
	
}
