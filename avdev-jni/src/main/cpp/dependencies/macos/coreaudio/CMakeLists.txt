cmake_minimum_required(VERSION 3.13)
project(avdev-coreaudio)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
	INTERFACE
		include/CoreAudioCaptureDevice.h
		include/CoreAudioInputStream.h
		include/CoreAudioManager.h
		include/CoreAudioOutputStream.h
		include/CoreAudioPlaybackDevice.h
		include/CoreAudioStream.h
		include/CoreAudioTypeConverter.h
		include/MacOSHelper.h
	PRIVATE
		src/CoreAudioCaptureDevice.cpp
		src/CoreAudioInputStream.cpp
		src/CoreAudioManager.cpp
		src/CoreAudioOutputStream.cpp
		src/CoreAudioPlaybackDevice.cpp
		src/CoreAudioStream.cpp
		src/CoreAudioTypeConverter.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
	include
)

target_compile_options(${PROJECT_NAME} PRIVATE -x objective-c++)