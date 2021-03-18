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

#include "Device.h"

namespace avdev
{
	Device::Device(std::string name, std::string descriptor) :
		name(name),
		descriptor(descriptor)
	{
	}

	bool Device::operator==(const Device & other)
	{
		return name == other.name && descriptor == other.descriptor;
	}
	
	bool Device::operator!=(const Device & other)
	{
		return !(*this == other);
	}
	
	bool Device::operator<(const Device & other)
	{
		return name < other.name;
	}

	std::string Device::getDescriptor() const
	{
		return descriptor;
	}

	std::string Device::getName() const
	{
		return name;
	}
}