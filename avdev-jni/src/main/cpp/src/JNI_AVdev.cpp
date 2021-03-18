#include "AVdevException.h"
#include "JNI_AVdevContext.h"
#include "JavaContext.h"
#include "JavaRuntimeException.h"
#include "Exception.h"

#include <jni.h>

jni::JavaContext * javaContext = nullptr;

/*
JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_AVdev_addLogger
(JNIEnv * env, jobject caller, jobject jlogger)
{
	try {
		// Connect Java logger with native logger implementation.
		jobject g_logger = env->NewGlobalRef(jlogger);
		std::shared_ptr<avdev::JNI_LogStream> logger = std::make_shared<avdev::JNI_LogStream>(g_logger, avdev::LogLevel::Debug, 0);
	
		avdev::Log::addOutputStream(logger);
	}
	catch (avdev::Exception & ex) {
		THROW_JAVA_EXCEPTION(Classes.RuntimeException, ex.what());
	}
	catch (...) {
		THROW_UNKNOWN_EXCEPTION(Classes.RuntimeException);
	}
}
*/

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM * vm, void * reserved)
{
	JNIEnv * env = nullptr;

	if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
		return -1;
	}

	javaContext = new avdev::JNI_AVdevContext(vm);

	try {
		javaContext->initialize(env);
	}
	catch (avdev::AVdevException & ex) {
		env->Throw(jni::JavaRuntimeException(env, ex.what()));
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}

	return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM * vm, void * reserved)
{
	if (javaContext != nullptr) {
		JNIEnv * env = nullptr;

		if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
			return;
		}

		try {
			javaContext->destroy(env);
		}
		catch (...) {
			ThrowCxxJavaException(env);
		}

		delete javaContext;
	}
}
