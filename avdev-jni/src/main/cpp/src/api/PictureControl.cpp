#include "PictureControl.h"
#include "api/PictureControl.h"
#include "JavaClasses.h"
#include "JavaEnums.h"
#include "JavaString.h"
#include "JavaObject.h"
#include "JNI_AVdev.h"

namespace jni
{
	namespace PictureControl
	{
		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::PictureControl & nativeType)
		{
			const auto javaClass = JavaClasses::get<JavaPictureControlClass>(env);

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

		JavaPictureControlClass::JavaPictureControlClass(JNIEnv * env)
		{
			cls = FindClass(env, PKG "PictureControl");

			ctor = GetMethod(env, cls, "<init>", "(L" PKG "PictureControlType;JJJJZ)V");
		}
	}
}