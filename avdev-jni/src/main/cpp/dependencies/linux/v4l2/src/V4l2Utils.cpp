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

#include "V4l2Utils.h"

namespace avdev
{
	namespace v4l2
	{
		bool isV4l2Device(const char * name)
		{
			return !memcmp(name, "video", 5) || !memcmp(name, "radio", 5) ||
					!memcmp(name, "vbi", 3) || !memcmp(name, "v4l-subdev", 10);
		}
		
		int ioctlDevice(int fh, int request, void * arg)
		{
			int r;

			do {
				r = ioctl(fh, request, arg);
			}
			while (-1 == r && ((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

			return r;
		}
		
		int openDevice(const char * path, int oflags)
		{
			int fd = open(path, oflags);
			return fd;
		}
		
		int closeDevice(int fd)
		{
			int ret = close(fd);
			return ret;
		}
	}
}
