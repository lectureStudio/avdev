#ifndef AVDEV_JNI_AUDIO_SESSION_LISTENER_H_
#define AVDEV_JNI_AUDIO_SESSION_LISTENER_H_

#include "AudioSessionListener.h"
#include "JavaClass.h"
#include "JavaRef.h"

#include <jni.h>

namespace avdev
{
	class JNI_AudioSessionListener : public AudioSessionListener
	{
		public:
			JNI_AudioSessionListener(JNIEnv * env, const jni::JavaGlobalRef<jobject> & listener);
			~JNI_AudioSessionListener() { };

			void volumeChanged(float volume, bool mute) override;

		private:
			class JavaAudioSessionListenerClass : public jni::JavaClass
			{
				public:
					explicit JavaAudioSessionListenerClass(JNIEnv * env);

					jmethodID volumeChanged;
			};

		private:
			jni::JavaGlobalRef<jobject> listener;

			const std::shared_ptr<JavaAudioSessionListenerClass> javaClass;
	};


	using PJNIAudioSessionListener = std::shared_ptr<JNI_AudioSessionListener>;
}

#endif