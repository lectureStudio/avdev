#include "AudioPlaybackDevice.h"
#include "api/AudioPlaybackDevice.h"
#include "JavaClasses.h"
#include "JavaString.h"
#include "JavaUtils.h"
#include "JNI_AVdev.h"

namespace jni
{
	namespace AudioPlaybackDevice
	{
		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::PDevice & nativeType)
		{
			const auto javaClass = JavaClasses::get<JavaAudioPlaybackDeviceClass>(env);

			jobject obj = env->NewObject(javaClass->cls, javaClass->ctor);

			env->SetObjectField(obj, javaClass->nameId, JavaString::toJava(env, nativeType->getName()).get());
			env->SetObjectField(obj, javaClass->descId, JavaString::toJava(env, nativeType->getDescriptor()).get());

			SetHandle(env, obj, nativeType.get());

			return JavaLocalRef<jobject>(env, obj);
		}

		JavaAudioPlaybackDeviceClass::JavaAudioPlaybackDeviceClass(JNIEnv * env)
		{
			cls = FindClass(env, PKG "AudioPlaybackDevice");

			ctor = GetMethod(env, cls, "<init>", "()V");

			nameId = GetFieldID(env, cls, "name", STRING_SIG);
			descId = GetFieldID(env, cls, "descriptor", STRING_SIG);
		}
	}
}