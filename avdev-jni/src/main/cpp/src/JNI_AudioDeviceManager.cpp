#include "AVdevException.h"
#include "JNI_AudioDeviceManager.h"
#include "JNI_AVdevContext.h"
#include "AudioManager.h"
#include "AudioCaptureDevice.h"
#include "AudioPlaybackDevice.h"
#include "api/AudioCaptureDevice.h"
#include "api/AudioPlaybackDevice.h"
#include "JavaArrayList.h"
#include "JavaRuntimeException.h"

using namespace avdev;

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_AudioDeviceManager_getDefaultAudioCaptureDevice
(JNIEnv * env, jobject caller)
{
	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);
	AudioManager * manager = context->getAudioManager();

	try {
		PAudioCaptureDevice defaultCapture = manager->getDefaultAudioCaptureDevice();

		if (defaultCapture) {
			jni::JavaLocalRef<jobject> dev = jni::AudioCaptureDevice::toJava(env, defaultCapture);

			return dev.release();
		}
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return nullptr;
}

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_AudioDeviceManager_getDefaultAudioPlaybackDevice
(JNIEnv * env, jobject caller)
{
	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);
	AudioManager * manager = context->getAudioManager();

	try {
		PAudioPlaybackDevice defaultPlayback = manager->getDefaultAudioPlaybackDevice();

		if (defaultPlayback) {
			jni::JavaLocalRef<jobject> dev = jni::AudioPlaybackDevice::toJava(env, defaultPlayback);

			return dev.release();
		}
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return nullptr;
}

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_AudioDeviceManager_getAudioCaptureDevices
(JNIEnv * env, jobject caller)
{
	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);
	AudioManager * manager = context->getAudioManager();

	try {
		std::set<PAudioCaptureDevice> devices = manager->getAudioCaptureDevices();
		jsize count = static_cast<jsize>(devices.size());
		jni::JavaArrayList devList(env, count);

		for (auto & dev : devices) {
			devList.add(jni::AudioCaptureDevice::toJava(env, dev));
		}

		return devList.listObject().release();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return nullptr;
}

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_AudioDeviceManager_getAudioPlaybackDevices
(JNIEnv * env, jobject caller)
{
	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);
	AudioManager * manager = context->getAudioManager();

	try {
		std::set<PAudioPlaybackDevice> devices = manager->getAudioPlaybackDevices();
		jsize count = static_cast<jsize>(devices.size());
		jni::JavaArrayList devList(env, count);

		for (auto & dev : devices) {
			devList.add(jni::AudioPlaybackDevice::toJava(env, dev));
		}

		return devList.listObject().release();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return nullptr;
}
