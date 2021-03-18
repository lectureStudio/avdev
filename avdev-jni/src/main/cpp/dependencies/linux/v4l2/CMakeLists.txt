cmake_minimum_required(VERSION 3.13)
project(avdev-v4l2)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
	INTERFACE
		include/avdev-v4l2.h
		include/JpegDecoder.h
		include/V4l2TypeConverter.h
		include/V4l2Utils.h
		include/V4l2VideoCaptureDevice.h
		include/V4l2VideoManager.h
		include/V4l2VideoOutputStream.h
	PRIVATE
		src/JpegDecoder.cpp
		src/V4l2TypeConverter.cpp
		src/V4l2Utils.cpp
		src/V4l2VideoCaptureDevice.cpp
		src/V4l2VideoManager.cpp
		src/V4l2VideoOutputStream.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
	include
)
