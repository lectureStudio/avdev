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

#ifndef AVDEV_CORE_LOG_STREAM_H_
#define AVDEV_CORE_LOG_STREAM_H_

#include "avdev.h"
#include "LogLocation.h"

#include <iomanip>
#include <ostream>
#include <stdarg.h>
#include <string>
#include <thread>
#include <mutex>

const unsigned int MAX_LOG_MESSAGE_LEN = 1024;

namespace avdev
{
	enum class LogLevel
	{
		Debug,
		Info,
		Warn,
		Error,
		Fatal
	};

	enum class LogItem
	{
		LogLevel   = 0x1,
		Filename   = 0x2,
		Method     = 0x4,
		LineNumber = 0x8,
		DateTime   = 0x10,
		ThreadId   = 0x20,
		LoggerName = 0x40,
		All        = 0x80
	};

	class LogStream
	{
		public:
			LogStream(LogLevel level, int logmask) : level(level), logmask(logmask)
			{
			}

			virtual ~LogStream()
			{
			}

			template <typename ...Args>
			void debug(LogLocation & location, const std::string & message, Args && ...args)
			{
				std::lock_guard<std::mutex> lock(mutex);

				writeLog(location, LogLevel::Debug, message, std::forward<Args>(args)...);
			}

			template <typename ...Args>
			void error(LogLocation & location, const std::string & message, Args && ...args)
			{
				std::lock_guard<std::mutex> lock(mutex);

				writeLog(location, LogLevel::Error, message, std::forward<Args>(args)...);
			}

			template <typename ...Args>
			void fatal(LogLocation & location, const std::string & message, Args && ...args)
			{
				std::lock_guard<std::mutex> lock(mutex);

				writeLog(location, LogLevel::Fatal, message, std::forward<Args>(args)...);
			}

			template <typename ...Args>
			void info(LogLocation & location, const std::string & message, Args && ...args)
			{
				std::lock_guard<std::mutex> lock(mutex);

				writeLog(location, LogLevel::Info, message, std::forward<Args>(args)...);
			}

			template <typename ...Args>
			void warn(LogLocation & location, const std::string & message, Args && ...args)
			{
				std::lock_guard<std::mutex> lock(mutex);

				writeLog(location, LogLevel::Warn, message, std::forward<Args>(args)...);
			}

			template <typename ...Args>
			void log(LogLocation & location, LogLevel level, const std::string & message, Args && ...args)
			{
				std::lock_guard<std::mutex> lock(mutex);

				writeLog(location, level, message, std::forward<Args>(args)...);
			}

		protected:
			virtual void writeLog(LogLocation & location, LogLevel level, const std::string  message, ...) = 0;

			std::string leveltoString(LogLevel level)
			{
				switch (level) {
					case LogLevel::Debug:
						return "DEBUG";
					case LogLevel::Error:
						return "ERROR";
					case LogLevel::Fatal:
						return "FATAL";
					case LogLevel::Info:
						return "INFO";
					case LogLevel::Warn:
						return "WARN";
					default:
						return "";
				}
			}

		protected:
			LogLevel level;
			int logmask;
			
		private:
			std::mutex mutex;
	};
}

#endif