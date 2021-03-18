#include "PictureFormat.h"
#include "api/PictureFormat.h"
#include "JavaClasses.h"
#include "JavaEnums.h"
#include "JavaString.h"
#include "JavaObject.h"
#include "JNI_AVdev.h"

namespace jni
{
	namespace PictureFormat
	{
		JavaLocalRef<jobject> toJava(JNIEnv * env, const avdev::PictureFormat & nativeType)
		{
			const auto javaClass = JavaClasses::get<JavaPictureFormatClass>(env);

			jobject pixFormat = env->CallStaticObjectMethod(javaClass->pixelFormatCls, javaClass->pixelFormatById,
				static_cast<int>(nativeType.getPixelFormat()));

			jobject obj = env->NewObject(javaClass->cls, javaClass->ctor,
				pixFormat,
				nativeType.getWidth(),
				nativeType.getHeight()
			);

			return JavaLocalRef<jobject>(env, obj);
		}

		avdev::PictureFormat toNative(JNIEnv * env, const JavaRef<jobject> & javaType)
		{
			const auto javaClass = JavaClasses::get<JavaPictureFormatClass>(env);

			JavaObject obj(env, javaType);
			JavaObject formatObj(env, obj.getObject(javaClass->format));

			avdev::PixelFormat pixelFormat = static_cast<avdev::PixelFormat>(formatObj.getInt(javaClass->pixelFormatId));

			return avdev::PictureFormat(obj.getInt(javaClass->width), obj.getInt(javaClass->height), pixelFormat);
		}

		JavaPictureFormatClass::JavaPictureFormatClass(JNIEnv * env)
		{
			cls = FindClass(env, PKG "PictureFormat");

			ctor = GetMethod(env, cls, "<init>", "(L" PKG "PictureFormat$PixelFormat;II)V");

			format = GetFieldID(env, cls, "format", "L" PKG "PictureFormat$PixelFormat;");
			width = GetFieldID(env, cls, "width", "I");
			height = GetFieldID(env, cls, "height", "I");

			pixelFormatCls = FindClass(env, PKG "PictureFormat$PixelFormat");
			pixelFormatById = GetStaticMethod(env, pixelFormatCls, "byId", "(I)L" PKG "PictureFormat$PixelFormat;");
			pixelFormatId = GetFieldID(env, pixelFormatCls, "id", "I");
		}
	}
}