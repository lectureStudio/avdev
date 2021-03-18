#include "AudioInputStream.h"
#include "AudioOutputStream.h"
#include "VideoManager.h"
#include "VideoOutputStream.h"
#include "JNI_AVdevContext.h"
#include "JNI_AVdev.h"
#include "JavaEnums.h"
#include "JavaFactories.h"
#include "JavaUtils.h"

#ifdef _WIN32
#include "MFAudioManager.h"
#include "MFVideoManager.h"
#include "WasapiAudioManager.h"
#include "WindowsHelper.h"
#endif
#ifdef __linux__
#include "PulseAudioManager.h"
#include "V4l2VideoManager.h"
#endif
#ifdef __APPLE__
#include "AVFVideoManager.h"
#include "CoreAudioManager.h"
#endif

namespace avdev
{
	JNI_AVdevContext::JNI_AVdevContext(JavaVM * vm) :
		jni::JavaContext(vm)
	{
	}

	AudioManager * JNI_AVdevContext::getAudioManager()
	{
		return audioManager.get();
	}

	VideoManager * JNI_AVdevContext::getVideoManager()
	{
		return videoManager.get();
	}

	void JNI_AVdevContext::initialize(JNIEnv * env)
	{
		jni::JavaContext::initialize(env);

		jni::JavaEnums::add<SampleFormat>(env, PKG "AudioFormat$SampleFormat");
		jni::JavaEnums::add<CameraControlType>(env, PKG "CameraControlType");
		jni::JavaEnums::add<PictureControlType>(env, PKG "PictureControlType");

		jni::JavaFactories::add<AudioInputStream>(env, PKG "AudioInputStream");
		jni::JavaFactories::add<AudioOutputStream>(env, PKG "AudioOutputStream");
		jni::JavaFactories::add<VideoOutputStream>(env, PKG "VideoOutputStream");

		//MessageQueue & mq = MessageQueue::instance();
		//mq.start();

#ifdef _WIN32
		audioManager = std::make_unique<WasapiAudioManager>();
		//audioManager = std::make_unique<MFAudioManager>();
		videoManager = std::make_unique<MFVideoManager>();
#endif
#ifdef __linux__
		audioManager = std::make_unique<PulseAudioManager>();
		videoManager = std::make_unique<V4l2VideoManager>();
#endif
#ifdef __APPLE__
		audioManager = std::make_unique<CoreAudioManager>();
		videoManager = std::make_unique<AVFVideoManager>();
#endif
	}

	void JNI_AVdevContext::destroy(JNIEnv * env)
	{
		sessionListeners.clear();
		hotplugListeners.clear();
		streamListeners.clear();

		//MessageQueue & messageQueue = MessageQueue::instance();
		//messageQueue.stop();

		audioManager = nullptr;
		videoManager = nullptr;

		jni::JavaContext::destroy(env);
	}
}
