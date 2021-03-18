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

#ifndef AVDEV_V4l2_UTILS_H_
#define AVDEV_V4l2_UTILS_H_

#include <cstdint>
#include <string>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace avdev
{
	namespace v4l2
	{
		enum class IOMethod
		{
			READ,
			MMAP,
			USERPTR
		};
		
		bool isV4l2Device(const char * name);
		int ioctlDevice(int fh, int request, void *arg);
		int openDevice(const char * path, int oflags);
		int closeDevice(int fd);
	}
}

#endif