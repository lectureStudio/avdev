#ifndef AVDEV_JNI_AVDEV_H_
#define AVDEV_JNI_AVDEV_H_

#include <jni.h>


#ifdef __cplusplus
extern "C" {
#endif
	/*
	* Class:     org_lecturestudio_avdev_AVdev
	* Method:    addLogger
	* Signature: (Lorg/lecturestudio/avdev/Logger;)V
	*/
	JNIEXPORT void JNICALL Java_org_lecturestudio_avdev_AVdev_addLogger
		(JNIEnv *, jobject, jobject);

#ifdef __cplusplus
}
#endif


#define PKG "org/lecturestudio/avdev/"

#define STRING_SIG "Ljava/lang/String;"

#endif