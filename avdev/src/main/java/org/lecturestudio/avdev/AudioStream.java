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

public abstract class AudioStream extends Stream {

	native public void attachSessionListener(AudioSessionListener listener) throws Exception;

	native public void detachSessionListener(AudioSessionListener listener) throws Exception;

	native public float getVolume();
	native public void setVolume(float volume);

	native public boolean getMute();
	native public void setMute(boolean mute);

	native public void setAudioFormat(AudioFormat format);
	native public AudioFormat getAudioFormat();

	native public void setBufferLatency(int latency);
	native public int getBufferLatency();
	
	native public int getStreamPosition();
	
}
