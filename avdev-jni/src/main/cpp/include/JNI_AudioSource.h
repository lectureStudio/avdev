#ifndef AVDEV_JNI_AUDIO_SOURCE_H_
#define AVDEV_JNI_AUDIO_SOURCE_H_

#include "AudioFormat.h"
#include "AudioSource.h"
#include "JavaClass.h"
#include "JavaRef.h"

#include <jni.h>
#include <cstdint>

namespace avdev
{
	class JNI_AudioSource : public AudioSource
	{
		public:
			JNI_AudioSource(JNIEnv * env, const jni::JavaGlobalRef<jobject> & source);
			~JNI_AudioSource();

			int read(std::uint8_t * data, size_t dataOffset, size_t length);

		private:
			class JavaAudioSourceClass : public jni::JavaClass
			{
				public:
					explicit JavaAudioSourceClass(JNIEnv* env);

					jmethodID read;
			};

		private:
			jni::JavaGlobalRef<jobject> source;

			jbyteArray buffer;

			const std::shared_ptr<JavaAudioSourceClass> javaClass;
	};
}

#endif