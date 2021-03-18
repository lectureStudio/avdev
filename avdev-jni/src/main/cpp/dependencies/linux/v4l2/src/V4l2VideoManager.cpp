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

#include "AVdevException.h"
#include "V4l2VideoManager.h"
#include "V4l2VideoCaptureDevice.h"

#include <set>

namespace avdev
{
	V4l2VideoManager::V4l2VideoManager()
	{
		udev = udev_new();

		if (!udev) {
			throw AVdevException("V4l2: Can't create udev.");
		}

		startThread();
	}

	V4l2VideoManager::~V4l2VideoManager()
	{
		stopThreadAndWait();

		udev_unref(udev);
	}

	void V4l2VideoManager::run()
	{
		struct udev_monitor * mon = udev_monitor_new_from_netlink(udev, "udev");

		if (!mon) {
			throw AVdevException("V4l2: Failed to init udev monitor.");
		}

		udev_monitor_filter_add_match_subsystem_devtype(mon, UDEV_SUBSYSTEM, NULL);
		udev_monitor_enable_receiving(mon);

		fd_set fds;
		struct timeval tv;
		struct udev_device * dev;
		int fd = udev_monitor_get_fd(mon);

		while (isRunning()) {
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			tv.tv_sec = 1;
			tv.tv_usec = 0;

			int ret = select(fd + 1, &fds, NULL, NULL, &tv);

			if (ret > 0 && FD_ISSET(fd, &fds)) {
				dev = udev_monitor_receive_device(mon);

				if (!dev) {
					printf("V4l2: No device received from udev monitor.\n");
					continue;
				}

				const char * subsystem = udev_device_get_subsystem(dev);

				if (strcmp(subsystem, UDEV_SUBSYSTEM) != 0) {
					udev_device_unref(dev);
					continue;
				}

				const char * action = udev_device_get_action(dev);
				const char * node = udev_device_get_devnode(dev);
				const char * name = udev_device_get_property_value(dev, "ID_V4L_PRODUCT");

				if (strcmp(action, UDEV_ADD) == 0) {
					addDevice(name, node);
				}
				else if (strcmp(action, UDEV_REMOVE) == 0) {
					removeDevice(name, node);
				}

				udev_device_unref(dev);
			}
		}

		udev_monitor_unref(mon);
	}

	std::set<PVideoCaptureDevice> V4l2VideoManager::getVideoCaptureDevices()
	{
		if (!captureDevices.empty()) {
			return captureDevices.devices();
		}

		struct udev_enumerate * enumerate = udev_enumerate_new(udev);
		udev_enumerate_add_match_subsystem(enumerate, UDEV_SUBSYSTEM);
		udev_enumerate_scan_devices(enumerate);

		struct udev_list_entry * udev_devices = udev_enumerate_get_list_entry(enumerate);
		struct udev_list_entry * dev_list_entry;
		struct udev_device * dev;
		struct v4l2_capability vcap;

		udev_list_entry_foreach(dev_list_entry, udev_devices) {
			const char * path = udev_list_entry_get_name(dev_list_entry);

			if (!path) {
				printf("V4l2: Failed to get device sys path.\n");
				continue;
			}

			dev = udev_device_new_from_syspath(udev, path);

			if (!dev) {
				printf("V4l2: Failed to get device from sys path %s.\n", path);
				continue;
			}

			const char * node = udev_device_get_devnode(dev);

			int v4l2_fd = v4l2::openDevice(node, O_RDONLY);
			if (v4l2_fd < 0) {
				printf("V4l2: Failed to open device: %s.\n", node);
				continue;
			}

			if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_QUERYCAP, &vcap) == -1) {
				printf("V4l2: Failed to query device caps: %s.\n", node);
				continue;
			}

			if (!(vcap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
				printf("V4l2: %s is not a video capture device.\n", node);
				continue;
			}

			v4l2::closeDevice(v4l2_fd);

			const char * name = (const char *) vcap.card;

			addDevice(name, node);

			udev_device_unref(dev);
		}

		udev_enumerate_unref(enumerate);

		return captureDevices.devices();
	}

	void V4l2VideoManager::addDevice(const char * longname, const char * descriptor)
	{
		std::string desc(descriptor);
		std::string name(longname);

		auto inputDevice = std::make_shared<V4l2VideoCaptureDevice>(name, desc);
		captureDevices.insertDevice(inputDevice);

		notifyDeviceConnected(inputDevice);
	}

	void V4l2VideoManager::removeDevice(const char * longname, const char * descriptor)
	{
	    std::string name(longname);

	    auto predicate = [name](const std::shared_ptr<VideoDevice> & dev) {
			return name == dev->getName();
		};

		PVideoCaptureDevice removed = captureDevices.removeDevice(predicate);

		if (removed) {
			notifyDeviceDisconnected(removed);
		}
	}
}
