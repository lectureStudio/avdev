#ifndef AVDEV_JNI_API_VIDEO_CAPTURE_DEVICE_H_
#define AVDEV_JNI_API_VIDEO_CAPTURE_DEVICE_H_

#include "JavaClass.h"
#include "JavaRef.h"

#include "Device.h"

#include <jni.h>

namespace jni
{
	namespace VideoCaptureDevice
	{
		class JavaVideoCaptureDeviceClass : public JavaClass
		{
			public:
				explicit JavaVideoCaptureDeviceClass(JNIEnv * env);

				jclass cls;
				jmethodID ctor;
				jfieldID nameId;
				jfieldID descId;
		};

		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::PDevice & nativeType);
	}
}

#endif