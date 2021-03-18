#ifndef AVDEV_JNI_API_AUDIO_FORMAT_H_
#define AVDEV_JNI_API_AUDIO_FORMAT_H_

#include "JavaClass.h"
#include "JavaRef.h"

#include "AudioFormat.h"

#include <jni.h>

namespace jni
{
	namespace AudioFormat
	{
		class JavaAudioFormatClass : public JavaClass
		{
			public:
				explicit JavaAudioFormatClass(JNIEnv * env);

				jclass cls;
				jmethodID ctor;
				jfieldID channels;
				jfieldID sampleFormat;
				jfieldID sampleRate;
		};

		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::AudioFormat & nativeType);

		avdev::AudioFormat toNative(JNIEnv * env, const JavaRef<jobject> & javaType);
	}
}

#endif