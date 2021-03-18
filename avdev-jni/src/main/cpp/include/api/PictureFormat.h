#ifndef AVDEV_JNI_API_PICTURE_FORMAT_H_
#define AVDEV_JNI_API_PICTURE_FORMAT_H_

#include "JavaClass.h"
#include "JavaRef.h"

#include "PictureFormat.h"

#include <jni.h>

namespace jni
{
	namespace PictureFormat
	{
		class JavaPictureFormatClass : public JavaClass
		{
			public:
				explicit JavaPictureFormatClass(JNIEnv * env);

				jclass cls;
				jmethodID ctor;
				jfieldID format;
				jfieldID width;
				jfieldID height;

				jclass pixelFormatCls;
				jmethodID pixelFormatById;
				jfieldID pixelFormatId;
		};

		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::PictureFormat & nativeType);

		avdev::PictureFormat toNative(JNIEnv * env, const JavaRef<jobject> & javaType);
	}
}

#endif