#include "JNI_LogStream.h"
#include "JNI_AVdev.h"
#include "JavaString.h"
#include "JavaUtils.h"
#include <sstream>

namespace avdev
{
	JNI_LogStream::JNI_LogStream(JNIEnv * env, const jni::JavaGlobalRef<jobject> & logger, LogLevel level, int logmask) :
		LogStream(level, logmask),
		logger(logger),
		javaClass(jni::JavaClasses::get<JavaLoggerClass>(env))
	{
	}

	JNI_LogStream::~JNI_LogStream()
	{
	}

	void JNI_LogStream::writeLog(LogLocation & location, LogLevel level, const std::string  message, ...)
	{
		JNIEnv * env = AttachCurrentThread();

		if (logger != nullptr) {
			if (level < this->level) {
				return;
			}

			char text[MAX_LOG_MESSAGE_LEN] = { 0 };
			va_list vl;
			va_start(vl, message);
			vsnprintf(text, MAX_LOG_MESSAGE_LEN, message.c_str(), vl);
			va_end(vl);

			std::ostringstream stream;

			if (logmask & static_cast<int>(LogItem::DateTime)) {
				std::time_t t = std::time(nullptr);
				std::tm tm = { 0 };

				LOCAL_TIME(&tm, &t);

				wchar_t str[128];
				if (std::wcsftime(str, sizeof(str), L"%B %d, %Y %H:%M:%S", &tm)) {
					stream << str;
				}
			}

			if (logmask & static_cast<int>(LogItem::ThreadId)) {
				write<const char *>(stream, " ");
				write<std::thread::id>(stream, std::this_thread::get_id());
			}

			if (logmask & static_cast<int>(LogItem::LogLevel)) {
				write<const char *>(stream, " ");
				write<const char *>(stream, leveltoString(level).c_str());
			}

			if (logmask & static_cast<int>(LogItem::Filename)) {
				write<const char *>(stream, " ");
				write<const char *>(stream, location.getFileName());
			}

			if (logmask & static_cast<int>(LogItem::Method)) {
				write<const char *>(stream, " ");
				write<const char *>(stream, location.getMethodName());
			}

			if (logmask & static_cast<int>(LogItem::LineNumber)) {
				write<const char *>(stream, ":");
				write<int>(stream, location.getLineNumber());
			}

			write<const char *>(stream, text);

			jni::JavaLocalRef<jstring> jMessage = jni::JavaString::toJava(env, stream.str());

			switch (level) {
				case LogLevel::Debug:
					env->CallVoidMethod(logger, javaClass->onDebug, jMessage.get());
					break;
				case LogLevel::Error:
					env->CallVoidMethod(logger, javaClass->onError, jMessage.get());
					break;
				case LogLevel::Fatal:
					env->CallVoidMethod(logger, javaClass->onFatal, jMessage.get());
					break;
				case LogLevel::Info:
					env->CallVoidMethod(logger, javaClass->onInfo, jMessage.get());
					break;
				case LogLevel::Warn:
					env->CallVoidMethod(logger, javaClass->onWarn, jMessage.get());
					break;
			}
		}
	}

	template <class T>
	void JNI_LogStream::write(std::ostringstream & stream, T data)
	{
		stream << data;
	}

	JNI_LogStream::JavaLoggerClass::JavaLoggerClass(JNIEnv* env)
	{
		jclass cls = FindClass(env, PKG "Logger");

		onDebug = GetMethod(env, cls, "debug", "(" STRING_SIG ")V");
		onError = GetMethod(env, cls, "error", "(" STRING_SIG ")V");
		onFatal = GetMethod(env, cls, "fatal", "(" STRING_SIG ")V");
		onInfo = GetMethod(env, cls, "info", "(" STRING_SIG ")V");
		onWarn = GetMethod(env, cls, "warn", "(" STRING_SIG ")V");
	}
}