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

#ifndef AVDEV_CORE_WINDOWS_HOTPLUG_NOTIFIER_H_
#define AVDEV_CORE_WINDOWS_HOTPLUG_NOTIFIER_H_

#include <Windows.h>
#include <cwchar>
#include <string>
#include <list>

namespace avdev
{
	class WinHotplugNotifier
	{
		public:
			WinHotplugNotifier(std::list<GUID> devInterfaces);
			virtual ~WinHotplugNotifier();

			WinHotplugNotifier(WinHotplugNotifier const &) = delete;
			void operator=(WinHotplugNotifier const &) = delete;

		protected:
			void start();
			void stop();

			virtual void onDeviceConnected(std::wstring symLink) = 0;
			virtual void onDeviceDisconnected(std::wstring symLink) = 0;

		private:
			bool initializeWindow();
			bool registerForDeviceNotification();

			static void onClose(HWND hWnd);
			static void onDeviceChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
			static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

			static DWORD WINAPI run(void * context);

			std::list<GUID> deviceInterfaces;
			std::list<HDEVNOTIFY> deviceNotifyHandles;

			HWND hWnd;
			HANDLE threadHandle;

			const WCHAR * CLASS_NAME = L"MMF Device Notification Class";
			const WCHAR * WINDOW_NAME = L"MMF Device Notification Application";
	};
}

#endif