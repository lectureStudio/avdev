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

import java.io.IOException;
import java.util.List;

import org.lecturestudio.avdev.AudioFormat.SampleFormat;

public class TestAudioDeviceManager {

	private static class StreamStateListener implements StreamListener {

		@Override
		public void streamOpened() {
			System.out.println("Stream opened");
		}

		@Override
		public void streamClosed() {
			System.out.println("Stream closed");
		}

		@Override
		public void streamStarted() {
			System.out.println("Stream started");
		}

		@Override
		public void streamStopped() {
			System.out.println("Stream stopped");
		}
		
		@Override
		public void streamEnded() {
			System.out.println("Stream ended");
		}
		
	}
	
	private final AudioFormat format = new AudioFormat(SampleFormat.S16LE, 24000, 1);
	
	private final StreamListener streamListener = new StreamStateListener();
	
	
	public static void main(String[] args) throws Exception {
		TestAudioDeviceManager test = new TestAudioDeviceManager();
		test.testListDevices();
//		test.testAudioRecording();
//		test.testAudioPlayback();
		test.testHotplutListener();
		
//		Thread.sleep(30000);
	}
	
	private void testAudioRecording() throws IOException {
		AudioSink sink = new FileAudioSink("audio.raw");
		
		AudioDeviceManager manager = AudioDeviceManager.getInstance();
		AudioCaptureDevice defaultCaptureDev = manager.getDefaultAudioCaptureDevice();
		AudioOutputStream stream = defaultCaptureDev.createOutputStream(sink);

		try {
			stream.attachStreamListener(streamListener);
			stream.setAudioFormat(format);
			stream.setBufferLatency(10);
			stream.open();

			System.out.println("Format: " + stream.getAudioFormat());
			System.out.println("Latency: " + stream.getBufferLatency());
			System.out.println("Mute: " + stream.getMute());
			System.out.println("Volume: " + stream.getVolume());

			stream.start();

			Thread.sleep(5000);

			stream.stop();
			stream.close();
			stream.dispose();
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}
	
	private void testAudioPlayback() throws IOException {
		AudioSource source = new FileAudioSource("audio.raw");
		
		AudioDeviceManager manager = AudioDeviceManager.getInstance();
		AudioPlaybackDevice defaultPlaybackDev = manager.getDefaultAudioPlaybackDevice();
		AudioInputStream stream = defaultPlaybackDev.createInputStream(source);

		try {
			stream.attachStreamListener(streamListener);
			stream.setAudioFormat(format);
			stream.open();
			stream.start();

			Thread.sleep(5000);

			stream.stop();
			stream.close();
			stream.dispose();
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}
	
	private void testListDevices() {
		AudioDeviceManager manager = AudioDeviceManager.getInstance();

		List<AudioCaptureDevice> inputDevices = manager.getAudioCaptureDevices();
		List<AudioPlaybackDevice> outputDevices = manager.getAudioPlaybackDevices();
		
		AudioCaptureDevice defaultInput = manager.getDefaultAudioCaptureDevice();
		AudioPlaybackDevice defaultOutput = manager.getDefaultAudioPlaybackDevice();

		if (inputDevices != null) {
			System.out.println("Audio Capture Devices:");
			
			for (AudioCaptureDevice dev : inputDevices) {
				if (dev.equals(defaultInput)) {
					System.out.print(" *");
				}
				System.out.println(" " + dev);
			}
			
			System.out.println();
		}
		if (outputDevices != null) {
			System.out.println("Audio Playback Devices:");
			
			for (AudioPlaybackDevice dev : outputDevices) {
				if (dev.equals(defaultOutput)) {
					System.out.print(" *");
				}
				System.out.println(" " + dev);
			}
		}
	}
	
	private void testHotplutListener() throws Exception {
		HotplugListener listener = new HotplugListener() {
			
			@Override
			public void deviceDisconnected(Device device) {
				System.out.println("Disconnected: " + device);
			}
			
			@Override
			public void deviceConnected(Device device) {
				System.out.println("Connected: " + device);
			}
		};
		
		AudioDeviceManager manager = AudioDeviceManager.getInstance();
		manager.attachHotplugListener(listener);
		
		Thread.sleep(2000);
		System.out.println("Remove hotplug listener");
		manager.detachHotplugListener(listener);
		
		Thread.sleep(2000);
		System.out.println("Add hotplug listener");
		manager.attachHotplugListener(listener);
	}

}
