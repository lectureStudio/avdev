#include "DeviceManager.h"
#include "JNI_AVdevContext.h"
#include "JNI_DeviceManager.h"
#include "JNI_HotplugListener.h"
#include "JavaRef.h"
#include "JavaUtils.h"

using namespace avdev;

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_DeviceManager_attachHotplugListener
(JNIEnv * env, jobject caller, jobject listener)
{
	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);

	// Connect Java callback with native listener implementation.
	jni::JavaGlobalRef<jobject> listenerRef = jni::JavaGlobalRef<jobject>(env, listener);
	PJNIHotplugListener hotplugListener = std::make_shared<JNI_HotplugListener>(env, listenerRef);

	// Keep a reference to the listener.
	jlong ptr = reinterpret_cast<jlong>(listener);
	context->hotplugListeners[ptr] = hotplugListener;

	context->getAudioManager()->attachHotplugListener(hotplugListener);
	context->getVideoManager()->attachHotplugListener(hotplugListener);
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_DeviceManager_detachHotplugListener
(JNIEnv * env, jobject caller, jobject listener)
{
	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);

	jlong ptr = reinterpret_cast<jlong>(listener);
	auto found = context->hotplugListeners.find(ptr);

	if (found != context->hotplugListeners.end()) {
		PJNIHotplugListener hotplugListener = found->second;
		context->getAudioManager()->detachHotplugListener(hotplugListener);
		context->getVideoManager()->detachHotplugListener(hotplugListener);
		context->hotplugListeners.erase(found);
	}
}