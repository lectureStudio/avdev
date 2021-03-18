#include "AudioFormat.h"
#include "api/AudioFormat.h"
#include "JavaClasses.h"
#include "JavaEnums.h"
#include "JavaString.h"
#include "JavaObject.h"
#include "JNI_AVdev.h"

namespace jni
{
	namespace AudioFormat
	{
		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::AudioFormat & nativeType)
		{
			const auto javaClass = JavaClasses::get<JavaAudioFormatClass>(env);

			jobject obj = env->NewObject(javaClass->cls, javaClass->ctor,
				jni::JavaEnums::toJava(env, nativeType.getSampleFormat()).get(),
				nativeType.getSampleRate(),
				nativeType.getChannels()
			);

			return JavaLocalRef<jobject>(env, obj);
		}

		avdev::AudioFormat toNative(JNIEnv * env, const JavaRef<jobject> & javaType)
		{
			const auto javaClass = JavaClasses::get<JavaAudioFormatClass>(env);

			JavaObject obj(env, javaType);

			jni::JavaLocalRef<jobject> formatRef = obj.getObject(javaClass->sampleFormat);
			avdev::SampleFormat sampleFormat = jni::JavaEnums::toNative<avdev::SampleFormat>(env, formatRef.get());

			return avdev::AudioFormat(sampleFormat, obj.getInt(javaClass->sampleRate), obj.getInt(javaClass->channels));
		}

		JavaAudioFormatClass::JavaAudioFormatClass(JNIEnv * env)
		{
			cls = FindClass(env, PKG "AudioFormat");

			ctor = GetMethod(env, cls, "<init>", "(L" PKG "AudioFormat$SampleFormat;II)V");

			channels = GetFieldID(env, cls, "channels", "I");
			sampleFormat = GetFieldID(env, cls, "sampleFormat", "L" PKG "AudioFormat$SampleFormat;");
			sampleRate = GetFieldID(env, cls, "sampleRate", "I");
		}
	}
}