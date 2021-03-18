cmake_minimum_required(VERSION 3.13)
project(avdev-wasapi)

add_library(${PROJECT_NAME} STATIC)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DUNICODE -D_UNICODE)

target_sources(${PROJECT_NAME}
	INTERFACE
		include/MFAudioResampler.h
		include/WasapiAudioCaptureDevice.h
		include/WasapiAudioInputStream.h
		include/WasapiAudioManager.h
		include/WasapiAudioOutputStream.h
		include/WasapiAudioPlaybackDevice.h
		include/WasapiAudioSessionEvents.h
		include/WasapiAudioStream.h
	PRIVATE
		src/MFAudioResampler.cpp
		src/WasapiAudioCaptureDevice.cpp
		src/WasapiAudioInputStream.cpp
		src/WasapiAudioManager.cpp
		src/WasapiAudioOutputStream.cpp
		src/WasapiAudioPlaybackDevice.cpp
		src/WasapiAudioSessionEvents.cpp
		src/WasapiAudioStream.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
	include
)
