cmake_minimum_required(VERSION 3.13)
project(avdev-core)

add_library(${PROJECT_NAME} STATIC)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DUNICODE -D_UNICODE)

target_sources(${PROJECT_NAME}
	INTERFACE
		include/AudioCaptureDevice.h
		include/AudioDevice.h
		include/AudioFormat.h
		include/AudioInputStream.h
		include/AudioManager.h
		include/AudioOutputStream.h
		include/AudioPlaybackDevice.h
		include/AudioSessionListener.h
		include/AudioSink.h
		include/AudioSource.h
		include/AudioStream.h
		include/AVdevException.h
		include/avdev.h
		include/CameraControl.h
		include/Device.h
		include/DeviceList.h
		include/DeviceManager.h
		include/HotplugListener.h
		include/ImageUtils.h
		include/Log.h
		include/LogLocation.h
		include/LogOutputStream.h
		include/LogStream.h
		include/MessageQueue.h
		include/PictureControl.h
		include/PictureFormat.h
		include/PixelFormatConverter.h
		include/Queue.h
		include/RingBuffer.h
		include/Stream.h
		include/StreamListener.h
		include/Thread.h
		include/Transform.h
		include/VideoCaptureDevice.h
		include/VideoControl.h
		include/VideoDevice.h
		include/VideoManager.h
		include/VideoOutputStream.h
		include/VideoSink.h
		include/VideoStream.h
	PRIVATE
		src/AudioCaptureDevice.cpp
		src/AudioDevice.cpp
		src/AudioFormat.cpp
		src/AudioInputStream.cpp
		src/AudioManager.cpp
		src/AudioOutputStream.cpp
		src/AudioPlaybackDevice.cpp
		src/AudioStream.cpp
		src/AVdevException.cpp
		src/CameraControl.cpp
		src/Device.cpp
		src/DeviceManager.cpp
		src/MessageQueue.cpp
		src/PictureControl.cpp
		src/PictureFormat.cpp
		src/PixelFormatConverter.cpp
		src/Stream.cpp
		src/Thread.cpp
		src/VideoCaptureDevice.cpp
		src/VideoDevice.cpp
		src/VideoManager.cpp
		src/VideoOutputStream.cpp
		src/VideoStream.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
	include
)

if(APPLE)
	target_compile_options(${PROJECT_NAME} PRIVATE -x objective-c++)
elseif(WIN32)
	target_sources(${PROJECT_NAME}
		INTERFACE
			include/windows/WindowsHelper.h
			include/windows/WinHotplugNotifier.h
		PRIVATE
			src/windows/WinHotplugNotifier.cpp
	)

	target_include_directories(${PROJECT_NAME} PUBLIC
		include/windows
	)
endif()
