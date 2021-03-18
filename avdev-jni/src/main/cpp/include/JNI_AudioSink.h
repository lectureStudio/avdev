#ifndef AVDEV_JNI_AUDIO_SINK_H_
#define AVDEV_JNI_AUDIO_SINK_H_

#include "AudioFormat.h"
#include "AudioSink.h"
#include "JavaClass.h"
#include "JavaRef.h"

#include <jni.h>
#include <cstdint>

namespace avdev
{
	class JNI_AudioSink : public AudioSink
	{
		public:
			JNI_AudioSink(JNIEnv * env, const jni::JavaGlobalRef<jobject> & sink);
			~JNI_AudioSink();

			void write(const std::uint8_t * data, size_t length, const AudioFormat & format);

		private:
			class JavaAudioSinkClass : public jni::JavaClass
			{
				public:
					explicit JavaAudioSinkClass(JNIEnv * env);

					jmethodID write;
			};

		private:
			jni::JavaGlobalRef<jobject> sink;

			jbyteArray buffer;

			const std::shared_ptr<JavaAudioSinkClass> javaClass;
	};
}

#endif