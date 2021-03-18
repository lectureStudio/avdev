#include "JNI_VideoSink.h"
#include "JNI_AVdev.h"
#include "JavaUtils.h"

namespace avdev
{
	JNI_VideoSink::JNI_VideoSink(JNIEnv * env, const jni::JavaGlobalRef<jobject> & sink) :
		sink(sink),
		buffer(nullptr),
		javaClass(jni::JavaClasses::get<JavaVideoSinkClass>(env))
	{
	}

	JNI_VideoSink::~JNI_VideoSink()
	{
		buffer = nullptr;
	}

	void JNI_VideoSink::writeVideoFrame(const std::uint8_t * data, size_t length, const PictureFormat & format)
	{
		JNIEnv * env = AttachCurrentThread();
		jsize size = static_cast<jsize>(length);

		if (buffer == nullptr) {
			buffer = reinterpret_cast<jbyteArray>(env->NewGlobalRef(env->NewByteArray(size * 2)));
		}

		env->SetByteArrayRegion(buffer, 0, size, (jbyte *) data);
		env->CallVoidMethod(sink, javaClass->write, buffer, size);
	}

	JNI_VideoSink::JavaVideoSinkClass::JavaVideoSinkClass(JNIEnv* env)
	{
		jclass cls = FindClass(env, PKG "VideoSink");

		write = GetMethod(env, cls, "write", "([BI)V");
	}
}