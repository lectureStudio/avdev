#ifndef AVDEV_JNI_HOTPLUG_LISTENER_H_
#define AVDEV_JNI_HOTPLUG_LISTENER_H_

#include "HotplugListener.h"
#include "Device.h"
#include "JavaClass.h"
#include "JavaRef.h"

#include <jni.h>

namespace avdev
{
	class JNI_HotplugListener : public HotplugListener
	{
		public:
			JNI_HotplugListener(JNIEnv * env, const jni::JavaGlobalRef<jobject> & listener);
			~JNI_HotplugListener();

			void deviceConnected(PDevice device);
			void deviceDisconnected(PDevice device);

		private:
			jni::JavaLocalRef<jobject> createJavaDevice(JNIEnv* env, PDevice device);

			class JavaHotplugListenerClass : public jni::JavaClass
			{
				public:
					explicit JavaHotplugListenerClass(JNIEnv * env);

					jmethodID onDeviceConnected;
					jmethodID onDeviceDisconnected;
			};

		private:
			jni::JavaGlobalRef<jobject> listener;

			const std::shared_ptr<JavaHotplugListenerClass> javaClass;
	};


	using PJNIHotplugListener = std::shared_ptr<JNI_HotplugListener>;
}

#endif