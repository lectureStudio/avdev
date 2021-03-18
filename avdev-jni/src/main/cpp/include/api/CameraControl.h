#ifndef AVDEV_JNI_API_CAMERA_CONTROL_H_
#define AVDEV_JNI_API_CAMERA_CONTROL_H_

#include "JavaClass.h"
#include "JavaRef.h"

#include "CameraControl.h"

#include <jni.h>

namespace jni
{
	namespace CameraControl
	{
		class JavaCameraControlClass : public JavaClass
		{
			public:
				explicit JavaCameraControlClass(JNIEnv * env);

				jclass cls;
				jmethodID ctor;
		};

		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::CameraControl & nativeType);
	}
}

#endif