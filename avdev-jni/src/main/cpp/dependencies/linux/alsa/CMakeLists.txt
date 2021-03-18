cmake_minimum_required(VERSION 3.13)
project(avdev-alsa)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
	INTERFACE
		include/AlsaAudioCaptureDevice.h
		include/AlsaAudioInputStream.h
		include/AlsaAudioManager.h
		include/AlsaAudioOutputStream.h
		include/AlsaAudioPlaybackDevice.h
		include/AlsaAudioStream.h
	PRIVATE
		src/AlsaAudioCaptureDevice.cpp
		src/AlsaAudioInputStream.cpp
		src/AlsaAudioManager.cpp
		src/AlsaAudioOutputStream.cpp
		src/AlsaAudioPlaybackDevice.cpp
		src/AlsaAudioStream.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
	include
)
