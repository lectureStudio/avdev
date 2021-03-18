/*
 * Copyright 2016 Alex Andres
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AVDEV_CORE_DEVICE_MANAGER_H_
#define AVDEV_CORE_DEVICE_MANAGER_H_

#include <list>
#include <memory>

#include "HotplugListener.h"

namespace avdev
{
	enum DeviceEvent
	{
		Connected,
		Disconnected
	};
	
	class DeviceManager
	{
		public:
			virtual ~DeviceManager() {};

			void attachHotplugListener(PHotplugListener listener);
			void detachHotplugListener(PHotplugListener listener);

		protected:
			void notifyDeviceConnected(PDevice device);
			void notifyDeviceDisconnected(PDevice device);

		private:
			void notifyListeners(PDevice device, DeviceEvent event);
		
			std::list<std::weak_ptr<HotplugListener>> hotplugListeners;
	};
}

#endif