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

#include "DeviceManager.h"
#include "MessageQueue.h"

namespace avdev
{
	void DeviceManager::attachHotplugListener(PHotplugListener listener)
	{
		hotplugListeners.push_back(listener);
	}

	void DeviceManager::detachHotplugListener(PHotplugListener listener)
	{
		hotplugListeners.remove_if([listener](std::weak_ptr<HotplugListener> p) {
			return !(p.owner_before(listener) || listener.owner_before(p));
		});
	}

	void DeviceManager::notifyDeviceConnected(PDevice device)
	{
		notifyListeners(device, DeviceEvent::Connected);
	}

	void DeviceManager::notifyDeviceDisconnected(PDevice device)
	{
		notifyListeners(device, DeviceEvent::Disconnected);
	}
	
	void DeviceManager::notifyListeners(PDevice device, DeviceEvent event)
	{
		for (auto i = hotplugListeners.begin(); i != hotplugListeners.end();) {
			if ((*i).expired()) {
				i = hotplugListeners.erase(i);
			}
			else {
				PHotplugListener listener = (*i).lock();
				
				MessageQueue & mq = MessageQueue::instance();

				if (event == DeviceEvent::Connected) {
					mq.dispatch([device, listener]() { listener->deviceConnected(device); });
				}
				else if (event == DeviceEvent::Disconnected) {
					mq.dispatch([device, listener]() { listener->deviceDisconnected(device); });
				}
				
				++i;
			}
		}
	}
}