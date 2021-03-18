#ifndef AVDEV_JNI_LOG_STREAM_H_
#define AVDEV_JNI_LOG_STREAM_H_

#include "LogStream.h"
#include "JavaClass.h"
#include "JavaRef.h"

#include <jni.h>

namespace avdev
{
	class JNI_LogStream : public LogStream
	{
		public:
			JNI_LogStream(JNIEnv * env, const jni::JavaGlobalRef<jobject> & logger, LogLevel level, int logmask);
			~JNI_LogStream();

			void writeLog(LogLocation & location, LogLevel level, const std::string  message, ...);

		private:
			template <class T>
			void write(std::ostringstream & stream, T data);

			class JavaLoggerClass : public jni::JavaClass
			{
				public:
					explicit JavaLoggerClass(JNIEnv* env);

					jmethodID onDebug;
					jmethodID onError;
					jmethodID onFatal;
					jmethodID onInfo;
					jmethodID onWarn;
			};

		private:
			jni::JavaGlobalRef<jobject> logger;

			const std::shared_ptr<JavaLoggerClass> javaClass;
	};
}

#endif