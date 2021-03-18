#ifndef AVDEV_JNI_AVDEV_CONTEXT_H
#define AVDEV_JNI_AVDEV_CONTEXT_H

#include "JavaContext.h"
#include "AudioManager.h"
#include "VideoManager.h"
#include "JNI_AudioSessionListener.h"
#include "JNI_HotplugListener.h"
#include "JNI_StreamListener.h"

#include <jni.h>
#include <map>
#include <memory>

namespace avdev
{
	class JNI_AVdevContext : public jni::JavaContext
	{
		public:
			JNI_AVdevContext(JavaVM * vm);
			~JNI_AVdevContext() = default;

			AudioManager * getAudioManager();
			VideoManager * getVideoManager();

			void initialize(JNIEnv * env) override;
			void destroy(JNIEnv * env) override;

		public:
			std::map<jlong, PJNIAudioSessionListener> sessionListeners;
			std::map<jlong, PJNIHotplugListener> hotplugListeners;
			std::map<jlong, PJNIStreamListener> streamListeners;

		private:
			std::unique_ptr<AudioManager> audioManager;
			std::unique_ptr<VideoManager> videoManager;
	};
}

#endif
