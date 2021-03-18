#include "AudioPlaybackDevice.h"
#include "JNI_AudioPlaybackDevice.h"
#include "JNI_AudioSource.h"
#include "JavaFactories.h"
#include "JavaRef.h"
#include "JavaUtils.h"

using namespace avdev;

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_AudioPlaybackDevice_createInputStream
(JNIEnv * env, jobject caller, jobject source)
{
	AudioPlaybackDevice * device = GetHandle<AudioPlaybackDevice>(env, caller);
	CHECK_HANDLEV(device, nullptr);

	// Connect Java sink with native sink implementation.
	jni::JavaGlobalRef<jobject> listenerRef = jni::JavaGlobalRef<jobject>(env, source);
	std::shared_ptr<JNI_AudioSource> audioSource = std::make_shared<JNI_AudioSource>(env, listenerRef);

	PAudioInputStream stream = device->createInputStream(audioSource);

	if (stream == nullptr) {
		return nullptr;
	}

	// Keep the stream in memory and delete later with JNI_Stream::dispose().
	return jni::JavaFactories::create(env, stream.release()).release();
}