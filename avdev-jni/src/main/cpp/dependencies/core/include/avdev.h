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

#ifndef AVDEV_CORE_AVDEV_H_
#define AVDEV_CORE_AVDEV_H_

#include <cstdint>
#include <vector>
#include <string>

#ifndef _MSC_VER
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

#ifdef _WIN32
#define vsnprintf(a, b, c, d) vsnprintf_s(a, b, c, d)
#define LOCAL_TIME(tm, time) localtime_s(tm, time)
#else
#define LOCAL_TIME(tm, time) localtime_r(time, tm)
#endif

using Bytes = std::uint8_t *;
using ByteBuffer = std::vector<std::uint8_t>;

#endif