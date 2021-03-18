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

#ifndef AVDEV_CORE_LOG_H_
#define AVDEV_CORE_LOG_H_

#include <memory>
#include <string>
#include <vector>
#include "LogLocation.h"
#include "LogStream.h"

#if !defined(LOGDEV_LOCATION)
#define LOGDEV_LOCATION avdev::LogLocation(__FILE__, __FUNCTION__, __LINE__)
#endif

#define LOGDEV_INFO(message,...) avdev::Log::info(LOGDEV_LOCATION, message, ##__VA_ARGS__);
#define LOGDEV_DEBUG(message,...) avdev::Log::debug(LOGDEV_LOCATION, message, ##__VA_ARGS__);
#define LOGDEV_WARN(message,...) avdev::Log::warn(LOGDEV_LOCATION, message, ##__VA_ARGS__);
#define LOGDEV_ERROR(message,...) avdev::Log::error(LOGDEV_LOCATION, message, ##__VA_ARGS__);
#define LOGDEV_FATAL(message,...) avdev::Log::fatal(LOGDEV_LOCATION, message, ##__VA_ARGS__);

namespace avdev
{
	using PLoggerStream = std::shared_ptr<LogStream>;


	class Log
	{
		public:
			~Log()
			{
				outputStreams.clear();
			}

			static Log & instance()
			{
				static Log instance;
				return instance;
			}

			static void addOutputStream(PLoggerStream stream)
			{
				instance().outputStreams.push_back(stream);
			}

			template <typename ...Args>
			static void debug(LogLocation location, const std::string & message, Args && ...args)
			{
				for (PLoggerStream stream : instance().outputStreams) {
					stream->debug(location, message, std::forward<Args>(args)...);
				}
			}

			template <typename ...Args>
			static void error(LogLocation location, const std::string & message, Args && ...args)
			{
				for (PLoggerStream stream : instance().outputStreams) {
					stream->error(location, message, std::forward<Args>(args)...);
				}
			}

			template <typename ...Args>
			static void fatal(LogLocation location, const std::string & message, Args && ...args)
			{
				for (PLoggerStream stream : instance().outputStreams) {
					stream->fatal(location, message, std::forward<Args>(args)...);
				}
			}

			template <typename ...Args>
			static void info(LogLocation location, const std::string & message, Args && ...args)
			{
				for (PLoggerStream stream : instance().outputStreams) {
					stream->info(location, message, std::forward<Args>(args)...);
				}
			}

			template <typename ...Args>
			static void warn(LogLocation location, const std::string & message, Args && ...args)
			{
				for (PLoggerStream stream : instance().outputStreams) {
					stream->warn(location, message, std::forward<Args>(args)...);
				}
			}

			template <typename ...Args>
			static void log(LogLocation location, LogLevel level, const std::string & message, Args && ...args)
			{
				for (PLoggerStream stream : instance().outputStreams) {
					stream->log(location, level, message, std::forward<Args>(args)...);
				}
			}

		private:
			Log() {};
			Log(const Log &) = delete;
			Log & operator=(const Log &) = delete;

			std::vector<PLoggerStream> outputStreams;
	};
}

#endif