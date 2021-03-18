cmake_minimum_required(VERSION 3.13)
project(avdev-pulse)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
	INTERFACE
		include/PulseAudioCaptureDevice.h
		include/PulseAudioDevice.h
		include/PulseAudioInputStream.h
		include/PulseAudioManager.h
		include/PulseAudioOutputStream.h
		include/PulseAudioPlaybackDevice.h
		include/PulseAudioStream.h
	PRIVATE
		src/PulseAudioCaptureDevice.cpp
		src/PulseAudioDevice.cpp
		src/PulseAudioInputStream.cpp
		src/PulseAudioManager.cpp
		src/PulseAudioOutputStream.cpp
		src/PulseAudioPlaybackDevice.cpp
		src/PulseAudioStream.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
	include
)
