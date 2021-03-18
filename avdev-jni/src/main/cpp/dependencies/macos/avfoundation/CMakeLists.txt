cmake_minimum_required(VERSION 3.13)
project(avdev-avfoundation)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
	INTERFACE
		include/AVFTypeConverter.h
		include/AVFVideoCaptureDevice.h
		include/AVFVideoFrameCallback.h
		include/AVFVideoManager.h
		include/AVFVideoOutputStream.h
		include/AVFVideoStreamDelegate.h
	PRIVATE
		src/AVFTypeConverter.mm
		src/AVFVideoCaptureDevice.mm
		src/AVFVideoManager.mm
		src/AVFVideoOutputStream.mm
		src/AVFVideoStreamDelegate.mm
)

target_include_directories(${PROJECT_NAME} PUBLIC
	include
)
