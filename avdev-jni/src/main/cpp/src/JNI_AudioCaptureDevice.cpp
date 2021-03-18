#include "AudioCaptureDevice.h"
#include "JNI_AudioCaptureDevice.h"
#include "JNI_AudioSink.h"
#include "JavaFactories.h"
#include "JavaRef.h"
#include "JavaUtils.h"

using namespace avdev;

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_AudioCaptureDevice_createOutputStream
(JNIEnv * env, jobject caller, jobject sink)
{
	AudioCaptureDevice * device = GetHandle<AudioCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, nullptr);

	// Connect Java sink with native sink implementation.
	jni::JavaGlobalRef<jobject> listenerRef = jni::JavaGlobalRef<jobject>(env, sink);
	std::shared_ptr<JNI_AudioSink> audioSink = std::make_shared<JNI_AudioSink>(env, listenerRef);

	PAudioOutputStream stream = device->createOutputStream(audioSink);

	if (stream == nullptr) {
		return nullptr;
	}

	// Keep the stream in memory and delete later with JNI_Stream::dispose().
	return jni::JavaFactories::create(env, stream.release()).release();
}