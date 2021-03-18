cmake_minimum_required(VERSION 3.13)
project(avdev)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

if(APPLE)
	set(CMAKE_CXX_VISIBILITY_PRESET hidden)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
	set(AVDEV_OS macos)
	add_subdirectory(dependencies/${AVDEV_OS}/avfoundation)
	add_subdirectory(dependencies/${AVDEV_OS}/coreaudio)
elseif(LINUX)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
	set(AVDEV_OS linux)
	add_subdirectory(dependencies/${AVDEV_OS}/pulse)
	add_subdirectory(dependencies/${AVDEV_OS}/v4l2)
elseif(WIN32)
	set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
	set(AVDEV_OS windows)
	add_subdirectory(dependencies/${AVDEV_OS}/mf)
	add_subdirectory(dependencies/${AVDEV_OS}/wasapi)
endif()

add_subdirectory(dependencies/jni-voithos)
add_subdirectory(dependencies/core)

get_target_property(JNI_INCLUDES jni-voithos INCLUDE_DIRECTORIES)
get_target_property(CORE_INCLUDES avdev-core INCLUDE_DIRECTORIES)

set_property(
	TARGET avdev-core
	APPEND PROPERTY INCLUDE_DIRECTORIES ${JNI_INCLUDES}
)

add_library(${PROJECT_NAME} SHARED)

target_sources(${PROJECT_NAME}
	INTERFACE
		include/JNI_AudioCaptureDevice.h
		include/JNI_AudioDeviceManager.h
		include/JNI_AudioPlaybackDevice.h
		include/JNI_AudioSessionListener.h
		include/JNI_AudioSink.h
		include/JNI_AudioSource.h
		include/JNI_AudioStream.h
		include/JNI_AVdev.h
		include/JNI_AVdevContext.h
		include/JNI_DeviceManager.h
		include/JNI_HotplugListener.h
		include/JNI_LogStream.h
		include/JNI_Stream.h
		include/JNI_StreamListener.h
		include/JNI_VideoCaptureDevice.h
		include/JNI_VideoDeviceManager.h
		include/JNI_VideoSink.h
		include/api/AudioCaptureDevice.h
		include/api/AudioFormat.h
		include/api/AudioPlaybackDevice.h
		include/api/CameraControl.h
		include/api/PictureControl.h
		include/api/PictureFormat.h
		include/api/VideoCaptureDevice.h
	PRIVATE
		src/JNI_AudioCaptureDevice.cpp
		src/JNI_AudioDeviceManager.cpp
		src/JNI_AudioPlaybackDevice.cpp
		src/JNI_AudioSessionListener.cpp
		src/JNI_AudioSink.cpp
		src/JNI_AudioSource.cpp
		src/JNI_AudioStream.cpp
		src/JNI_AVdev.cpp
		src/JNI_AVdevContext.cpp
		src/JNI_DeviceManager.cpp
		src/JNI_HotplugListener.cpp
		src/JNI_LogStream.cpp
		src/JNI_Stream.cpp
		src/JNI_StreamListener.cpp
		src/JNI_VideoCaptureDevice.cpp
		src/JNI_VideoDeviceManager.cpp
		src/JNI_VideoSink.cpp
		src/api/AudioCaptureDevice.cpp
		src/api/AudioFormat.cpp
		src/api/AudioPlaybackDevice.cpp
		src/api/CameraControl.cpp
		src/api/PictureControl.cpp
		src/api/PictureFormat.cpp
		src/api/VideoCaptureDevice.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
	include
)

if(APPLE)
	target_compile_options(${PROJECT_NAME} PRIVATE -x objective-c++)
	set(AVDEV_LINK_LIBS -ObjC "-framework AVFoundation" "-framework CoreAudio" "-framework AudioToolbox" "-framework CoreMedia" "-framework CoreServices" "-framework Foundation" "-framework CoreVideo")
	set(PLATFORM_LIB avdev-avfoundation avdev-coreaudio)
elseif(LINUX)
	set(AVDEV_LINK_LIBS X11 -static-libgcc -static-libstdc++)
	set(AVDEV_LINK_LIBS pulse udev pthread jpeg)
	set(PLATFORM_LIB avdev-pulse avdev-v4l2)
elseif(WIN32)
	target_link_directories(${PROJECT_NAME} PRIVATE dependencies/${AVDEV_OS}/lib/x86-64)
	set(AVDEV_LINK_LIBS avrt.lib mf.lib mfreadwrite.lib mfplat.lib mfuuid.lib shlwapi.lib strmiids.lib wmcodecdspuuid.lib)
	set(PLATFORM_LIB avdev-mf avdev-wasapi)
endif()

set_property(
	TARGET ${PLATFORM_LIB}
	APPEND PROPERTY INCLUDE_DIRECTORIES ${JNI_INCLUDES} ${CORE_INCLUDES}
)

target_link_libraries(${PROJECT_NAME} ${PLATFORM_LIB} ${AVDEV_LINK_LIBS} jni-voithos avdev-core)

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT Runtime
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT Runtime
)
