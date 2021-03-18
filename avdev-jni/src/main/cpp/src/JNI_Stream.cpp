#include "AVdevException.h"
#include "Stream.h"
#include "JNI_AVdevContext.h"
#include "JNI_Stream.h"
#include "JNI_StreamListener.h"
#include "JavaRuntimeException.h"
#include "JavaUtils.h"

using namespace avdev;

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_Stream_open
(JNIEnv * env, jobject caller)
{
	Stream * stream = GetHandle<Stream>(env, caller);
	CHECK_HANDLE(stream);

	try {
		stream->open();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_Stream_close
(JNIEnv * env, jobject caller)
{
	Stream * stream = GetHandle<Stream>(env, caller);
	CHECK_HANDLE(stream);

	try {
		stream->close();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_Stream_start
(JNIEnv * env, jobject caller)
{
	Stream * stream = GetHandle<Stream>(env, caller);
	CHECK_HANDLE(stream);

	try {
		stream->start();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_Stream_stop
(JNIEnv * env, jobject caller)
{
	Stream * stream = GetHandle<Stream>(env, caller);
	CHECK_HANDLE(stream);

	try {
		stream->stop();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_Stream_dispose
(JNIEnv * env, jobject caller)
{
	Stream * stream = GetHandle<Stream>(env, caller);
	CHECK_HANDLE(stream);

	try {
		delete stream;
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_Stream_attachStreamListener
(JNIEnv * env, jobject caller, jobject listener)
{
	Stream * stream = GetHandle<Stream>(env, caller);
	CHECK_HANDLE(stream);

	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);

	// Connect Java callback with native listener implementation.
	jni::JavaGlobalRef<jobject> listenerRef = jni::JavaGlobalRef<jobject>(env, listener);
	PJNIStreamListener streamListener = std::make_shared<JNI_StreamListener>(env, listenerRef);

	// Keep a reference to the listener.
	jlong ptr = reinterpret_cast<jlong>(listener);
	context->streamListeners[ptr] = streamListener;

	try {
		stream->attachStreamListener(streamListener);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_Stream_detachStreamListener
(JNIEnv * env, jobject caller, jobject jListener)
{
	Stream * stream = GetHandle<Stream>(env, caller);
	CHECK_HANDLE(stream);

	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);

	jlong ptr = reinterpret_cast<jlong>(jListener);
	auto found = context->streamListeners.find(ptr);

	if (found != context->streamListeners.end()) {
		PJNIStreamListener pListener = found->second;

		try {
			stream->detachStreamListener(pListener);
		}
		catch (AVdevException & ex) {
			env->Throw(jni::JavaRuntimeException(env, ex.what()));
		}

		context->streamListeners.erase(found);
	}
}