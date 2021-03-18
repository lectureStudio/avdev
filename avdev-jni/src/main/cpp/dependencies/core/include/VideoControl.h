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

#ifndef AVDEV_CORE_VIDEO_CONTROL_H_
#define AVDEV_CORE_VIDEO_CONTROL_H_

#include "avdev.h"

namespace avdev
{
	template <class T>
	class VideoControl
	{
		public:
			VideoControl(T type, long min, long max, long step, long def, bool autoMode) :
				type(type),
				minValue(min),
				maxValue(max),
				step(step),
				defValue(def),
				autoMode(autoMode)
			{
			}
			
			bool operator== (const VideoControl & other) const
			{
				return (type == other.type && minValue == other.minValue && maxValue == other.maxValue &&
						step == other.step && defValue == other.defValue && autoMode == other.autoMode);
			}
			
			bool operator!= (const VideoControl & other) const
			{
				return !(*this == other);
			}

			T getType() const
			{
				return type;
			}

			long getMinValue() const
			{
				return minValue;
			}

			long getMaxValue() const
			{
				return maxValue;
			}

			long getStepValue() const
			{
				return step;
			}

			long getDefaultValue() const
			{
				return defValue;
			}

			bool hasAutoMode() const
			{
				return autoMode;
			}

			std::string toString()
			{
				std::string str = getName();
				str += ": " + std::to_string(minValue) + ":" + std::to_string(step) + ":" + std::to_string(maxValue);
				str += ", Default Value: " + std::to_string(defValue);
				str += ", Auto-Mode: " + std::to_string(autoMode);

				return str;
			}
			
			virtual std::string getName() const = 0;
			
		protected:
			T type;
			long minValue;
			long maxValue;
			long step;
			long defValue;
			bool autoMode;
	};
}

#endif