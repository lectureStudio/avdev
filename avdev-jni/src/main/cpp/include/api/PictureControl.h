#ifndef AVDEV_JNI_API_PICTURE_CONTROL_H_
#define AVDEV_JNI_API_PICTURE_CONTROL_H_

#include "JavaClass.h"
#include "JavaRef.h"

#include "PictureControl.h"

#include <jni.h>

namespace jni
{
	namespace PictureControl
	{
		class JavaPictureControlClass : public JavaClass
		{
			public:
				explicit JavaPictureControlClass(JNIEnv * env);

				jclass cls;
				jmethodID ctor;
		};

		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::PictureControl & nativeType);
	}
}

#endif