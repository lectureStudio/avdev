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

#ifndef AVDEV_CORE_LOG_OUTPUT_STREAM_H_
#define AVDEV_CORE_LOG_OUTPUT_STREAM_H_

#include "LogStream.h"
#include <ostream>

namespace avdev
{
	class LogOutputStream : public LogStream
	{
		public:
			LogOutputStream(std::wostream * stream, LogLevel level, int logmask) : LogStream(level, logmask), stream(stream)
			{
			}

			virtual ~LogOutputStream()
			{
				//delete stream;
			}

		protected:
			void writeLog(LogLocation & location, LogLevel level, const std::string  message, ...)
			{
				if (level < this->level) {
					return;
				}

				char text[MAX_LOG_MESSAGE_LEN] = { 0 };
				va_list vl;
				va_start(vl, message);
				vsnprintf(text, MAX_LOG_MESSAGE_LEN, message.c_str(), vl);
				va_end(vl);

				if (logmask & static_cast<int>(LogItem::DateTime)) {
					writeDatetime();
				}

				if (logmask & static_cast<int>(LogItem::ThreadId)) {
					write<std::thread::id>(std::this_thread::get_id());
				}

				if (logmask & static_cast<int>(LogItem::LogLevel)) {
					write<const char *>(leveltoString(level).c_str());
				}

				if (logmask & static_cast<int>(LogItem::Filename)) {
					write<const char *>(location.getFileName());
				}

				if (logmask & static_cast<int>(LogItem::Method)) {
					write<const char *>(location.getMethodName());
				}

				if (logmask & static_cast<int>(LogItem::LineNumber)) {
					write<const char *>(":");
					write<int>(location.getLineNumber());
				}

				write<const char *>("-");
				write<const char *>(text);

				(*stream) << std::endl;
				stream->flush();
			}

		private:
			template <class T>
			void write(T data)
			{
				(*stream) << " " << data;
			}

			void writeDatetime()
			{
				std::time_t t = std::time(nullptr);
				std::tm tm = { 0 };

				LOCAL_TIME(&tm, &t);

				wchar_t str[128];
				if (std::wcsftime(str, sizeof(str), L"%B %d, %Y %H:%M:%S", &tm)) {
					(*stream) << str;
				}
			}

		private:
			std::wostream * stream;
	};
}

#endif