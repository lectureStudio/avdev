#ifndef AVDEV_JNI_STREAM_LISTENER_H_
#define AVDEV_JNI_STREAM_LISTENER_H_

#include "StreamListener.h"
#include "JavaClass.h"
#include "JavaRef.h"

#include <jni.h>

namespace avdev
{
	class JNI_StreamListener : public StreamListener
	{
		public:
			JNI_StreamListener(JNIEnv * env, const jni::JavaGlobalRef<jobject> & listener);
			~JNI_StreamListener();

			void streamOpened() override;
			void streamClosed() override;
			void streamStarted() override;
			void streamStopped() override;
			void streamEnded() override;

		private:
			void notify(const jmethodID & method);

			class JavaStreamListenerClass : public jni::JavaClass
			{
				public:
					explicit JavaStreamListenerClass(JNIEnv * env);

					jmethodID onStreamOpened;
					jmethodID onStreamClosed;
					jmethodID onStreamStarted;
					jmethodID onStreamStopped;
					jmethodID onStreamEnded;
			};

		private:
			jni::JavaGlobalRef<jobject> listener;

			const std::shared_ptr<JavaStreamListenerClass> javaClass;
	};


	using PJNIStreamListener = std::shared_ptr<JNI_StreamListener>;
}

#endif