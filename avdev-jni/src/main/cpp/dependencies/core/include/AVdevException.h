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

#ifndef AVDEV_CORE_AVDEV_EXCEPTION_H_
#define AVDEV_CORE_AVDEV_EXCEPTION_H_

#include <stdarg.h>
#include <string>

#ifndef _MSC_VER
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

namespace avdev
{
	class AVdevException : public std::exception
	{
		public:
			AVdevException();
			AVdevException(const char * msg, ...);

			virtual ~AVdevException() throw() {}

			virtual const char * what() const NOEXCEPT;

		protected:
			std::string message;
	};
}

#endif