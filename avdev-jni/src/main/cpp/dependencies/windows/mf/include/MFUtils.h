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

#ifndef AVDEV_MF_UTILS_H_
#define AVDEV_MF_UTILS_H_

#include <string>
#include <Mfidl.h>

namespace avdev
{
	namespace mmf
	{
		void CreateAudioMediaType(GUID subType, UINT32 sampleRate, UINT32 channels, UINT32 bitsPerSample, IMFMediaType ** mediaType);

		void CreateVideoMediaType(GUID subType, UINT32 width, UINT32 height, UINT32 numerator, UINT32 denominator, IMFMediaType ** mediaType);

		void AddSourceNode(IMFTopology * topology, IMFMediaSource * source, IMFPresentationDescriptor * pd, IMFStreamDescriptor * sd, WCHAR * mmcssClass, IMFTopologyNode ** node);

		void AddOutputNode(IMFTopology * topology, IMFActivate * activate, DWORD sinkId, IMFTopologyNode ** node);

		void CreateSinkActivate(IMFSampleGrabberSinkCallback * callback, IMFMediaType * type, IMFActivate ** sinkActivate);

		void CreateSinkActivate(std::string endpointID, IMFActivate ** sinkActivate);

		void CreateSinkActivate(HWND videoWnd, IMFActivate ** sinkActivate);

		void CreateMediaSession(IMFMediaSession ** session);

		void CreateMediaSource(GUID mediaType, std::string symlink, IMFMediaSource ** source);

		void CreateTopology(GUID mediaType, IMFMediaSource * source, IMFActivate * sinkActivate, WCHAR * mmcssClass, IMFTopology ** topology);

		void GetMediaTypeHandler(IMFMediaSource * source, DWORD streamIndex, IMFMediaTypeHandler ** typeHandler);

		template <class T>
		HRESULT GetTopologyNodeObject(IMFTopologyNode * node, T ** object);

		HRESULT BindOutputNode(IMFTopologyNode * node);

		void BindOutputNodes(IMFTopology * topology);

		void ListTopologyTransforms(IMFTopology * topology);

		HRESULT QueueEventWithIUnknown(IMFMediaEventGenerator * gen, MediaEventType eventType, HRESULT status, IUnknown * unk);
	}
}

#endif