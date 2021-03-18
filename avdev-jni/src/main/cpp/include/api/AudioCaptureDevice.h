#ifndef AVDEV_JNI_API_AUDIO_CAPTURE_DEVICE_H_
#define AVDEV_JNI_API_AUDIO_CAPTURE_DEVICE_H_

#include "JavaClass.h"
#include "JavaRef.h"

#include "Device.h"

#include <jni.h>

namespace jni
{
	namespace AudioCaptureDevice
	{
		class JavaAudioCaptureDeviceClass : public JavaClass
		{
			public:
				explicit JavaAudioCaptureDeviceClass(JNIEnv * env);

				jclass cls;
				jmethodID ctor;
				jfieldID nameId;
				jfieldID descId;
		};

		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::PDevice & nativeType);
	}
}

#endif