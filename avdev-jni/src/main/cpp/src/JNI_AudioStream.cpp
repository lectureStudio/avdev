#include "AVdevException.h"
#include "AudioStream.h"
#include "api/AudioFormat.h"
#include "JNI_AVdevContext.h"
#include "JNI_AudioStream.h"
#include "JNI_AudioSessionListener.h"
#include "JavaRuntimeException.h"
#include "JavaUtils.h"

using namespace avdev;

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_AudioStream_attachSessionListener
(JNIEnv* env, jobject caller, jobject listener)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLE(stream);

	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);

	// Connect Java callback with native listener implementation.
	jni::JavaGlobalRef<jobject> listenerRef = jni::JavaGlobalRef<jobject>(env, listener);
	PJNIAudioSessionListener sessionListener = std::make_shared<JNI_AudioSessionListener>(env, listenerRef);

	// Keep a reference to the listener.
	jlong ptr = reinterpret_cast<jlong>(listener);
	context->sessionListeners[ptr] = sessionListener;

	try {
		stream->attachSessionListener(sessionListener);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_AudioStream_detachSessionListener
(JNIEnv* env, jobject caller, jobject jListener)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLE(stream);

	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);

	jlong ptr = reinterpret_cast<jlong>(jListener);
	auto found = context->sessionListeners.find(ptr);

	if (found != context->sessionListeners.end()) {
		PJNIAudioSessionListener pListener = found->second;

		try {
			stream->detachSessionListener(pListener);
		}
		catch (AVdevException & ex) {
			env->Throw(jni::JavaRuntimeException(env, ex.what()));
		}

		context->sessionListeners.erase(found);
	}
}

JNIEXPORT jfloat JNICALL Java_org_lecturestudio_avdev_AudioStream_getVolume
(JNIEnv * env, jobject caller)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLEV(stream, 0);

	float volume = 0;

	try {
		volume = stream->getVolume();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}

	return volume;
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_AudioStream_setVolume
(JNIEnv * env, jobject caller, jfloat volume)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLE(stream);

	try {
		stream->setVolume(volume);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
}

JNIEXPORT jboolean JNICALL Java_org_lecturestudio_avdev_AudioStream_getMute
(JNIEnv * env, jobject caller)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLEV(stream, false);

	bool mute = false;

	try {
		mute = stream->getMute();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}

	return mute;
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_AudioStream_setMute
(JNIEnv * env, jobject caller, jboolean mute)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLE(stream);

	// Avoid performance warning C4800.
	bool m = (mute == JNI_TRUE);

	try {
		stream->setMute(m);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_AudioStream_setAudioFormat
(JNIEnv * env, jobject caller, jobject format)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLE(stream);

	try {
		AudioFormat fmt = jni::AudioFormat::toNative(env, jni::JavaLocalRef<jobject>(env, format));

		stream->setAudioFormat(fmt);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
}

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_AudioStream_getAudioFormat
(JNIEnv * env, jobject caller)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLEV(stream, nullptr);

	try {
		AudioFormat fmt = stream->getAudioFormat();

		return jni::AudioFormat::toJava(env, fmt).release();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}

	return nullptr;
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_AudioStream_setBufferLatency
(JNIEnv * env, jobject caller, jint latency)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLE(stream);

	try {
		stream->setBufferLatency(latency);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
}

JNIEXPORT jint JNICALL Java_org_lecturestudio_avdev_AudioStream_getBufferLatency
(JNIEnv * env, jobject caller)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLEV(stream, 0);

	int latency = 0;

	try {
		latency = stream->getBufferLatency();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}

	return latency;
}

JNIEXPORT jint JNICALL Java_org_lecturestudio_avdev_AudioStream_getStreamPosition
(JNIEnv * env, jobject caller)
{
	AudioStream * stream = GetHandle<AudioStream>(env, caller);
	CHECK_HANDLEV(stream, 0);

	jint position = 0;

	try {
		position = static_cast<jint>(stream->getStreamPosition());
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}

	return position;
}