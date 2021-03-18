#ifndef AVDEV_JNI_VIDEO_SINK_H_
#define AVDEV_JNI_VIDEO_SINK_H_

#include "PictureFormat.h"
#include "VideoSink.h"
#include "JavaClass.h"
#include "JavaRef.h"

#include <jni.h>
#include <cstdint>

namespace avdev
{
	class JNI_VideoSink : public VideoSink
	{
		public:
			JNI_VideoSink(JNIEnv * env, const jni::JavaGlobalRef<jobject> & sink);
			~JNI_VideoSink();

			void writeVideoFrame(const std::uint8_t * data, size_t length, const PictureFormat & format);

		private:
			class JavaVideoSinkClass : public jni::JavaClass
			{
				public:
					explicit JavaVideoSinkClass(JNIEnv * env);

					jmethodID write;
			};

		private:
			jni::JavaGlobalRef<jobject> sink;

			jbyteArray buffer;

			const std::shared_ptr<JavaVideoSinkClass> javaClass;
	};
}

#endif