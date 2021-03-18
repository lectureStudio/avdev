#include "AudioCaptureDevice.h"
#include "AudioPlaybackDevice.h"
#include "VideoCaptureDevice.h"
#include "api/AudioCaptureDevice.h"
#include "api/AudioPlaybackDevice.h"
#include "api/VideoCaptureDevice.h"
#include "JNI_AVdev.h"
#include "JNI_HotplugListener.h"
#include "JavaUtils.h"

namespace avdev
{
	JNI_HotplugListener::JNI_HotplugListener(JNIEnv * env, const jni::JavaGlobalRef<jobject> & listener) :
		listener(listener),
		javaClass(jni::JavaClasses::get<JavaHotplugListenerClass>(env))
	{
	}

	JNI_HotplugListener::~JNI_HotplugListener()
	{
	}

	void JNI_HotplugListener::deviceConnected(PDevice device)
	{
		JNIEnv * env = AttachCurrentThread();
		jni::JavaLocalRef<jobject> dev = createJavaDevice(env, device);

		env->CallVoidMethod(listener, javaClass->onDeviceConnected, dev.get());
	}

	void JNI_HotplugListener::deviceDisconnected(PDevice device)
	{
		JNIEnv * env = AttachCurrentThread();
		jni::JavaLocalRef<jobject> dev = createJavaDevice(env, device);

		env->CallVoidMethod(listener, javaClass->onDeviceDisconnected, dev.get());
	}

	jni::JavaLocalRef<jobject> JNI_HotplugListener::createJavaDevice(JNIEnv * env, PDevice device)
	{
		jni::JavaLocalRef<jobject> dev = nullptr;

		if (dynamic_cast<AudioCaptureDevice *>(device.get())) {
			dev = jni::AudioCaptureDevice::toJava(env, device);
		}
		else if (dynamic_cast<AudioPlaybackDevice *>(device.get())) {
			dev = jni::AudioPlaybackDevice::toJava(env, device);
		}
		else if (dynamic_cast<VideoCaptureDevice *>(device.get())) {
			dev = jni::VideoCaptureDevice::toJava(env, device);
		}

		return dev;
	}

	JNI_HotplugListener::JavaHotplugListenerClass::JavaHotplugListenerClass(JNIEnv * env)
	{
		jclass cls = FindClass(env, PKG "HotplugListener");

		onDeviceConnected = GetMethod(env, cls, "deviceConnected", "(L" PKG "Device;)V");
		onDeviceDisconnected = GetMethod(env, cls, "deviceDisconnected", "(L" PKG "Device;)V");
	}
}
