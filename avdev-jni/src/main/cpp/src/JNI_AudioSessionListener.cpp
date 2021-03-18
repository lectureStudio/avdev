#include "JNI_AVdev.h"
#include "JNI_AudioSessionListener.h"
#include "JavaUtils.h"

namespace avdev
{
	JNI_AudioSessionListener::JNI_AudioSessionListener(JNIEnv * env, const jni::JavaGlobalRef<jobject> & listener) :
		listener(listener),
		javaClass(jni::JavaClasses::get<JavaAudioSessionListenerClass>(env))
	{
	}

	void JNI_AudioSessionListener::volumeChanged(float volume, bool mute)
	{
		JNIEnv * env = AttachCurrentThread();

		env->CallVoidMethod(listener, javaClass->volumeChanged, volume, mute);
	}

	JNI_AudioSessionListener::JavaAudioSessionListenerClass::JavaAudioSessionListenerClass(JNIEnv * env)
	{
		jclass cls = FindClass(env, PKG "AudioSessionListener");

		volumeChanged = GetMethod(env, cls, "volumeChanged", "(FZ)V");
	}
}
