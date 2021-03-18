#include "AVdevException.h"
#include "JNI_AVdevContext.h"
#include "JNI_VideoDeviceManager.h"
#include "api/VideoCaptureDevice.h"
#include "JavaArrayList.h"
#include "JavaRef.h"
#include "JavaRuntimeException.h"
#include "VideoManager.h"
#include "VideoCaptureDevice.h"

using namespace avdev;

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_VideoDeviceManager_getDefaultVideoCaptureDevice
(JNIEnv * env, jobject caller)
{
	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);
	VideoManager * manager = context->getVideoManager();

	try {
		PVideoCaptureDevice defaultCapture = manager->getDefaultVideoCaptureDevice();

		if (defaultCapture) {
			jni::JavaLocalRef<jobject> dev = jni::VideoCaptureDevice::toJava(env, defaultCapture);

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

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_VideoDeviceManager_getVideoCaptureDevices
(JNIEnv * env, jobject caller)
{
	JNI_AVdevContext * context = static_cast<JNI_AVdevContext *>(javaContext);
	VideoManager * manager = context->getVideoManager();

	try {
		std::set<PVideoCaptureDevice> devices = manager->getVideoCaptureDevices();
		jsize count = static_cast<jsize>(devices.size());
		jni::JavaArrayList devList(env, count);

		for (auto & dev : devices) {
			devList.add(jni::VideoCaptureDevice::toJava(env, dev));
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