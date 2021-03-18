#include "JNI_AudioSink.h"
#include "JNI_AVdev.h"
#include "JavaUtils.h"

namespace avdev
{
	JNI_AudioSink::JNI_AudioSink(JNIEnv * env, const jni::JavaGlobalRef<jobject> & sink) :
		sink(sink),
		buffer(nullptr),
		javaClass(jni::JavaClasses::get<JavaAudioSinkClass>(env))
	{
	}

	JNI_AudioSink::~JNI_AudioSink()
	{
		buffer = nullptr;
	}

	void JNI_AudioSink::write(const std::uint8_t * data, size_t length, const AudioFormat & format)
	{
		JNIEnv * env = AttachCurrentThread();
		jsize size = static_cast<jsize>(length);

		if (buffer == nullptr) {
			buffer = reinterpret_cast<jbyteArray>(env->NewGlobalRef(env->NewByteArray(size * 2)));
		}

		env->SetByteArrayRegion(buffer, 0, size, (jbyte *) data);
		env->CallVoidMethod(sink, javaClass->write, buffer, size);
	}

	JNI_AudioSink::JavaAudioSinkClass::JavaAudioSinkClass(JNIEnv * env)
	{
		jclass cls = FindClass(env, PKG "AudioSink");

		write = GetMethod(env, cls, "write", "([BI)V");
	}
}