cmake_minimum_required(VERSION 3.13)
project(avdev-mf)

add_library(${PROJECT_NAME} STATIC)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DUNICODE -D_UNICODE)

target_sources(${PROJECT_NAME}
	INTERFACE
		include/MFAudioCaptureDevice.h
		include/MFAudioInputStream.h
		include/MFAudioManager.h
		include/MFAudioOutputStream.h
		include/MFAudioPlaybackDevice.h
		include/MFInitializer.h
		include/MFOutputStream.h
		include/MFPcmMediaSource.h
		include/MFStream.h
		include/MFTypeConverter.h
		include/MFUtils.h
		include/MFVideoCaptureDevice.h
		include/MFVideoManager.h
		include/MFVideoOutputStream.h
	PRIVATE
		src/MFAudioCaptureDevice.cpp
		src/MFAudioInputStream.cpp
		src/MFAudioManager.cpp
		src/MFAudioOutputStream.cpp
		src/MFAudioPlaybackDevice.cpp
		src/MFInitializer.cpp
		src/MFOutputStream.cpp
		src/MFPcmMediaSource.cpp
		src/MFStream.cpp
		src/MFTypeConverter.cpp
		src/MFUtils.cpp
		src/MFVideoCaptureDevice.cpp
		src/MFVideoManager.cpp
		src/MFVideoOutputStream.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
	include
)
