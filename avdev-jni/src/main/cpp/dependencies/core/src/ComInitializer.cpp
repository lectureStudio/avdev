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

#include "ComInitializer.h"
#include "WindowsHelper.h"

namespace avdev
{
	ComInitializer::ComInitializer() : initialized(false)
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

		if (hr != RPC_E_CHANGED_MODE) {
			THROW_IF_FAILED(hr, "Initialize COM failed.");

			initialized = true;
		}
	}

	ComInitializer::~ComInitializer()
	{
		if (initialized) {
			CoUninitialize();
		}
	}

	bool ComInitializer::isInitialized()
	{
		return initialized;
	}
}