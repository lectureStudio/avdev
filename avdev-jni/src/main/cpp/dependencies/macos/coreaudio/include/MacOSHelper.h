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

#ifndef AVDEV_MAC_OS_HELPER_H_
#define AVDEV_MAC_OS_HELPER_H_

#include "AVdevException.h"
#include "Log.h"

#include <string>

#define THROW_IF_FAILED(status, msg, ...) ThrowIfFailed(LOGDEV_LOCATION, status, msg, ##__VA_ARGS__)

inline void ThrowIfFailed(avdev::LogLocation loc, OSStatus status, const char * msg, ...) {
	if (status != noErr) {
		char message[256];
		
		va_list args;
		va_start(args, msg);
		vsnprintf(message, 256, msg, args);
		va_end(args);
		
		throw avdev::AVdevException("%s Status: %d \n (%s, %s : %d)",
							   message, status, loc.getFileName(), loc.getMethodName(), loc.getLineNumber());
	}
}

#endif