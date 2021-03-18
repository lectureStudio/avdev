#include "JNI_AudioSource.h"
#include "JNI_AVdev.h"
#include "JavaUtils.h"

namespace avdev {

	JNI_AudioSource::JNI_AudioSource(JNIEnv * env, const jni::JavaGlobalRef<jobject> & source) :
		source(source),
		buffer(nullptr),
		javaClass(jni::JavaClasses::get<JavaAudioSourceClass>(env))
	{
	}

	JNI_AudioSource::~JNI_AudioSource()
	{
		buffer = nullptr;
	}

	int JNI_AudioSource::read(std::uint8_t * data, size_t dataOffset, size_t length)
	{
		JNIEnv * env = AttachCurrentThread();
		jsize size = static_cast<jsize>(length);

		if (buffer == nullptr) {
			buffer = reinterpret_cast<jbyteArray>(env->NewGlobalRef(env->NewByteArray(1024 * 1024)));
		}

		int read = env->CallIntMethod(source, javaClass->read, buffer, 0, size);

		if (read > 0) {
			env->GetByteArrayRegion(buffer, 0, size, reinterpret_cast<jbyte *>(data));
		}

		return read;
	}

	JNI_AudioSource::JavaAudioSourceClass::JavaAudioSourceClass(JNIEnv * env)
	{
		jclass cls = FindClass(env, PKG "AudioSource");

		read = GetMethod(env, cls, "read", "([BII)I");
	}
}