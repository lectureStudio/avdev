#include "AVdevException.h"
#include "VideoCaptureDevice.h"
#include "api/CameraControl.h"
#include "api/PictureControl.h"
#include "api/PictureFormat.h"
#include "JNI_VideoCaptureDevice.h"
#include "JNI_VideoSink.h"
#include "JavaArrayList.h"
#include "JavaEnums.h"
#include "JavaFactories.h"
#include "JavaRuntimeException.h"
#include "JavaUtils.h"
#include <limits>

using namespace avdev;

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_getPictureFormats
(JNIEnv * env, jobject caller)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, nullptr);

	try {
		std::list<PictureFormat> picFormats = device->getPictureFormats();
		jsize count = static_cast<jsize>(picFormats.size());
		jni::JavaArrayList formatList(env, count);

		for (const PictureFormat & format : picFormats) {
			formatList.add(jni::PictureFormat::toJava(env, format));
		}

		return formatList.listObject().release();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return nullptr;
}

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_createOutputStream
(JNIEnv * env, jobject caller, jobject sink)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, nullptr);

	// Connect Java sink with native sink implementation.
	jni::JavaGlobalRef<jobject> listenerRef = jni::JavaGlobalRef<jobject>(env, sink);
	std::shared_ptr<JNI_VideoSink> videoSink = std::make_shared<JNI_VideoSink>(env, listenerRef);

	PVideoOutputStream stream = device->createOutputStream(videoSink);

	if (stream == nullptr) {
		return nullptr;
	}

	// Keep the stream in memory and delete later with JNI_Stream::dispose().
	return jni::JavaFactories::create(env, stream.release()).release();
}

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_getPictureControls
(JNIEnv * env, jobject caller)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, nullptr);

	try {
		std::list<PictureControl> picControls = device->getPictureControls();
		jsize count = static_cast<jsize>(picControls.size());
		jni::JavaArrayList controlList(env, count);

		for (const PictureControl & control : picControls) {
			controlList.add(jni::PictureControl::toJava(env, control));
		}

		return controlList.listObject().release();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return nullptr;
}

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_getCameraControls
(JNIEnv * env, jobject caller)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, nullptr);

	try {
		std::list<CameraControl> camControls = device->getCameraControls();
		jsize count = static_cast<jsize>(camControls.size());
		jni::JavaArrayList controlList(env, count);

		for (const CameraControl & control : camControls) {
			controlList.add(jni::CameraControl::toJava(env, control));
		}

		return controlList.listObject().release();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return nullptr;
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_setPictureControlAutoMode
(JNIEnv * env, jobject caller, jobject type, jboolean autoMode)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLE(device);

	try {
		PictureControlType ctrlType = jni::JavaEnums::toNative<PictureControlType>(env, type);
		bool mode = (autoMode == JNI_TRUE);

		device->setPictureControlAutoMode(ctrlType, mode);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}
}

JNIEXPORT jboolean JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_getPictureControlAutoMode
(JNIEnv * env, jobject caller, jobject type)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, 0);

	try {
		PictureControlType ctrlType = jni::JavaEnums::toNative<PictureControlType>(env, type);
		bool autoMode = device->getPictureControlAutoMode(ctrlType);

		return (autoMode == JNI_TRUE);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return false;
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_setPictureControlValue
(JNIEnv * env, jobject caller, jobject type, jlong value)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLE(device);

	try {
		PictureControlType ctrlType = jni::JavaEnums::toNative<PictureControlType>(env, type);

		device->setPictureControlValue(ctrlType, static_cast<long>(value));
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}
}

JNIEXPORT jlong JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_getPictureControlValue
(JNIEnv * env, jobject caller, jobject type)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, 0);

	try {
		PictureControlType ctrlType = jni::JavaEnums::toNative<PictureControlType>(env, type);

		return device->getPictureControlValue(ctrlType);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return std::numeric_limits<long>::min();
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_setCameraControlAutoMode
(JNIEnv * env, jobject caller, jobject type, jboolean autoMode)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLE(device);

	try {
		CameraControlType ctrlType = jni::JavaEnums::toNative<CameraControlType>(env, type);
		bool mode = (autoMode == JNI_TRUE);

		device->setCameraControlAutoMode(ctrlType, mode);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}
}

JNIEXPORT jboolean JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_getCameraControlAutoMode
(JNIEnv * env, jobject caller, jobject type)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, 0);

	try {
		CameraControlType ctrlType = jni::JavaEnums::toNative<CameraControlType>(env, type);
		bool autoMode = device->getCameraControlAutoMode(ctrlType);

		return (autoMode == JNI_TRUE);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return false;
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_setCameraControlValue
(JNIEnv * env, jobject caller, jobject type, jlong value)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLE(device);

	try {
		CameraControlType ctrlType = jni::JavaEnums::toNative<CameraControlType>(env, type);

		device->setCameraControlValue(ctrlType, static_cast<long>(value));
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}
}

JNIEXPORT jlong JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_getCameraControlValue
(JNIEnv * env, jobject caller, jobject type)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, 0);

	try {
		CameraControlType ctrlType = jni::JavaEnums::toNative<CameraControlType>(env, type);

		return device->getCameraControlValue(ctrlType);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return std::numeric_limits<long>::min();
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_setFrameRate
(JNIEnv * env, jobject caller, jfloat frameRate)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLE(device);

	try {
		device->setFrameRate(frameRate);
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}
}

JNIEXPORT jfloat JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_getFrameRate
(JNIEnv * env, jobject caller)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, 0);

	try {
		return device->getFrameRate();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return 0;
}

JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_setPictureFormat
(JNIEnv * env, jobject caller, jobject format)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLE(device);

	try {
		jni::JavaLocalRef<jobject> javaRef = jni::JavaLocalRef<jobject>(env, format);

		device->setPictureFormat(jni::PictureFormat::toNative(env, javaRef));
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}
}

JNIEXPORT jobject JNICALL Java_org_lecturestudio_avdev_VideoCaptureDevice_getPictureFormat
(JNIEnv * env, jobject caller)
{
	VideoCaptureDevice * device = GetHandle<VideoCaptureDevice>(env, caller);
	CHECK_HANDLEV(device, nullptr);

	try {
		return jni::PictureFormat::toJava(env, device->getPictureFormat()).release();
	}
	catch (AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return nullptr;
}