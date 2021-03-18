#include "CameraControl.h"
#include "api/CameraControl.h"
#include "JavaClasses.h"
#include "JavaEnums.h"
#include "JavaString.h"
#include "JavaObject.h"
#include "JNI_AVdev.h"

namespace jni
{
	namespace CameraControl
	{
		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::CameraControl & nativeType)
		{
			const auto javaClass = JavaClasses::get<JavaCameraControlClass>(env);

			jobject obj = env->NewObject(javaClass->cls, javaClass->ctor,
				jni::JavaEnums::toJava(env, nativeType.getType()).get(),
				static_cast<jlong>(nativeType.getMinValue()),
				static_cast<jlong>(nativeType.getMaxValue()),
				static_cast<jlong>(nativeType.getStepValue()),
				static_cast<jlong>(nativeType.getDefaultValue()),
				static_cast<jboolean>(nativeType.hasAutoMode())
			);

			return JavaLocalRef<jobject>(env, obj);
		}

		JavaCameraControlClass::JavaCameraControlClass(JNIEnv * env)
		{
			cls = FindClass(env, PKG "CameraControl");

			ctor = GetMethod(env, cls, "<init>", "(L" PKG "CameraControlType;JJJJZ)V");
		}
	}
}