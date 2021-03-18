#include "JNI_StreamListener.h"
#include "JNI_AVdev.h"
#include "JavaUtils.h"

namespace avdev
{
	JNI_StreamListener::JNI_StreamListener(JNIEnv * env, const jni::JavaGlobalRef<jobject> & listener) :
		listener(listener),
		javaClass(jni::JavaClasses::get<JavaStreamListenerClass>(env))
	{
	}

	JNI_StreamListener::~JNI_StreamListener()
	{
	}

	void JNI_StreamListener::streamOpened()
	{
		notify(javaClass->onStreamOpened);
	}

	void JNI_StreamListener::streamClosed()
	{
		notify(javaClass->onStreamClosed);
	}

	void JNI_StreamListener::streamStarted()
	{
		notify(javaClass->onStreamStarted);
	}

	void JNI_StreamListener::streamStopped()
	{
		notify(javaClass->onStreamStopped);
	}

	void JNI_StreamListener::streamEnded()
	{
		notify(javaClass->onStreamEnded);
	}

	void JNI_StreamListener::notify(const jmethodID & method)
	{
		JNIEnv * env = AttachCurrentThread();

		env->CallVoidMethod(listener, method);
	}

	JNI_StreamListener::JavaStreamListenerClass::JavaStreamListenerClass(JNIEnv * env)
	{
		jclass cls = FindClass(env, PKG "StreamListener");

		onStreamOpened = GetMethod(env, cls, "streamOpened", "()V");
		onStreamClosed = GetMethod(env, cls, "streamClosed", "()V");
		onStreamStarted = GetMethod(env, cls, "streamStarted", "()V");
		onStreamStopped = GetMethod(env, cls, "streamStopped", "()V");
		onStreamEnded = GetMethod(env, cls, "streamEnded", "()V");
	}
}