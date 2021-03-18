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

#include "MFUtils.h"
#include "WindowsHelper.h"
#include "ComPtr.h"

#include <VersionHelpers.h>

namespace avdev
{
	namespace mmf
	{
		void CreateAudioMediaType(GUID subType, UINT32 sampleRate, UINT32 channels, UINT32 bitsPerSample, IMFMediaType ** mediaType)
		{
			jni::ComPtr<IMFMediaType> type;

			HRESULT hr = MFCreateMediaType(&type);
			THROW_IF_FAILED(hr, "MMF: Create media type failed.");

			hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			THROW_IF_FAILED(hr, "MMF: Set major media type failed.");

			hr = type->SetGUID(MF_MT_SUBTYPE, subType);
			THROW_IF_FAILED(hr, "MMF: Set media sub-type failed.");

			hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channels);
			THROW_IF_FAILED(hr, "MMF: Set audio channels failed.");

			hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
			THROW_IF_FAILED(hr, "MMF: Set sample rate failed.");

			hr = type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, (bitsPerSample / 8) * channels);
			THROW_IF_FAILED(hr, "MMF: Set block alignment failed.");

			hr = type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
			THROW_IF_FAILED(hr, "MMF: Set bits per sample failed.");

			hr = type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, 1);
			THROW_IF_FAILED(hr, "MMF: Set all samples independent failed.");

			hr = type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, (bitsPerSample / 8) * channels * sampleRate);
			THROW_IF_FAILED(hr, "MMF: Set average bytes per second failed.");

			*mediaType = type;
			(*mediaType)->AddRef();
		}

		void CreateVideoMediaType(GUID subType, UINT32 width, UINT32 height, UINT32 numerator, UINT32 denominator, IMFMediaType ** mediaType)
		{
			jni::ComPtr<IMFMediaType> type;

			HRESULT hr = MFCreateMediaType(&type);
			THROW_IF_FAILED(hr, "MMF: Create media type failed.");

			hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			THROW_IF_FAILED(hr, "MMF: Set major media type failed.");

			hr = type->SetGUID(MF_MT_SUBTYPE, subType);
			THROW_IF_FAILED(hr, "MMF: Set media sub-type failed.");

			hr = MFSetAttributeSize(type, MF_MT_FRAME_SIZE, width, height);
			THROW_IF_FAILED(hr, "MMF: Set frame size failed.");

			hr = MFSetAttributeRatio(type, MF_MT_FRAME_RATE, numerator, denominator);
			THROW_IF_FAILED(hr, "MMF: Set frame rate failed.");

			*mediaType = type;
			(*mediaType)->AddRef();
		}

		void AddSourceNode(IMFTopology * topology, IMFMediaSource * source, IMFPresentationDescriptor * pd, IMFStreamDescriptor * sd, WCHAR * mmcssClass, IMFTopologyNode ** node)
		{
			UINT32 flags = MF_CONNECT_DIRECT | MF_CONNECT_ALLOW_CONVERTER | MF_CONNECT_ALLOW_DECODER;
			jni::ComPtr<IMFTopologyNode> pNode;
			HRESULT hr;

			hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
			THROW_IF_FAILED(hr, "MMF: Create topology node failed.");
			
			hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, source);
			THROW_IF_FAILED(hr, "MMF: Node set source failed.");

			hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pd);
			THROW_IF_FAILED(hr, "MMF: Node set presentation descriptor failed.");

			hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, sd);
			THROW_IF_FAILED(hr, "MMF: Node set stream descriptor failed.");

			hr = pNode->SetUINT32(MF_TOPONODE_CONNECT_METHOD, flags);
			THROW_IF_FAILED(hr, "MMF: Node set connect method failed.");

			// Set attributes for the use of the Multimedia Class Scheduler Service (MMCSS).
			hr = pNode->SetUINT32(MF_TOPONODE_WORKQUEUE_ID, 0);
			THROW_IF_FAILED(hr, "MMF: Node set work queue ID failed.");

			/*
			Audio
			Capture
			Distribution
			Games
			Playback
			Pro Audio
			Window Manager
			*/

			hr = pNode->SetString(MF_TOPONODE_WORKQUEUE_MMCSS_CLASS, mmcssClass);
			THROW_IF_FAILED(hr, "MMF: Node set work queue MMCSS class failed.");

			hr = topology->AddNode(pNode);
			THROW_IF_FAILED(hr, "MMF: Topology add node failed.");

			*node = pNode;
			(*node)->AddRef();
		}

		void AddOutputNode(IMFTopology * topology, IMFActivate * activate, DWORD sinkId, IMFTopologyNode ** node)
		{
			UINT32 flags = MF_CONNECT_DIRECT | MF_CONNECT_ALLOW_CONVERTER | MF_CONNECT_ALLOW_DECODER;
			jni::ComPtr<IMFTopologyNode> pNode;
			HRESULT hr;

			hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
			THROW_IF_FAILED(hr, "MMF: Create topology node failed.");
			
			hr = pNode->SetObject(activate);
			THROW_IF_FAILED(hr, "MMF: Node set activate failed.");
			
			hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, sinkId);
			THROW_IF_FAILED(hr, "MMF: Node set stream ID failed.");
			
			hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
			THROW_IF_FAILED(hr, "MMF: Node set shutdown on remove failed.");

			hr = pNode->SetUINT32(MF_TOPONODE_CONNECT_METHOD, flags);
			THROW_IF_FAILED(hr, "MMF: Node set connect method failed.");

			hr = topology->AddNode(pNode);
			THROW_IF_FAILED(hr, "MMF: Topology add node failed.");

			*node = pNode;
			(*node)->AddRef();
		}

		void CreateSinkActivate(IMFSampleGrabberSinkCallback * callback, IMFMediaType * type, IMFActivate ** sinkActivate)
		{
			jni::ComPtr<IMFActivate> activate;
			HRESULT hr;

			hr = MFCreateSampleGrabberSinkActivate(type, callback, &activate);
			THROW_IF_FAILED(hr, "MMF: Create sample grabber sink activate failed.");

			if (IsWindows7OrGreater()) {
				// To run as fast as possible, set this attribute (requires Windows 7).
				hr = activate->SetUINT32(MF_SAMPLEGRABBERSINK_IGNORE_CLOCK, TRUE);
				THROW_IF_FAILED(hr, "MMF: Sink activate set ignore clock failed.");
			}

			*sinkActivate = activate;
			(*sinkActivate)->AddRef();
		}

		void CreateSinkActivate(std::string endpointID, IMFActivate ** sinkActivate)
		{
			jni::ComPtr<IMFActivate> pActivate;

			HRESULT hr = MFCreateAudioRendererActivate(&pActivate);
			THROW_IF_FAILED(hr, "MMF: Create audio renderer activate failed.");

			if (IsWindows8OrGreater()) {
				hr = pActivate->SetUINT32(MF_LOW_LATENCY, TRUE);
				THROW_IF_FAILED(hr, "MMF: Activate enable low-latency failed.");
			}

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			std::wstring endpoint = converter.from_bytes(endpointID);

			hr = pActivate->SetString(MF_AUDIO_RENDERER_ATTRIBUTE_ENDPOINT_ID, endpoint.c_str());
			THROW_IF_FAILED(hr, "MMF: Activate set endpoint ID failed.");

			*sinkActivate = pActivate;
			(*sinkActivate)->AddRef();
		}

		void CreateSinkActivate(HWND videoWnd, IMFActivate ** sinkActivate)
		{
			jni::ComPtr<IMFActivate> pActivate;

			HRESULT hr = MFCreateVideoRendererActivate(videoWnd, &pActivate);
			THROW_IF_FAILED(hr, "MMF: Create video renderer activate failed.");

			*sinkActivate = pActivate;
			(*sinkActivate)->AddRef();
		}

		void CreateMediaSession(IMFMediaSession ** session)
		{
			jni::ComPtr<IMFMediaSession> mediaSession;
			jni::ComPtr<IMFAttributes> sessionAttributes;
			HRESULT hr;

			if (IsWindows8OrGreater()) {
				hr = MFCreateAttributes(&sessionAttributes, 1);
				THROW_IF_FAILED(hr, "MMF: Create attributes failed.");

				// Enable low-latency processing.
				hr = sessionAttributes->SetUINT32(MF_LOW_LATENCY, 1);
				THROW_IF_FAILED(hr, "MMF: Session enable low-latency processing failed.");
			}

			hr = MFCreateMediaSession(sessionAttributes, &mediaSession);
			THROW_IF_FAILED(hr, "MMF: Create media session failed.");

			*session = mediaSession;
			(*session)->AddRef();
		}

		void CreateMediaSource(GUID mediaType, std::string symlink, IMFMediaSource ** source)
		{
			jni::ComPtr<IMFMediaSource> mediaSource;
			jni::ComPtr<IMFAttributes> attributes;
			GUID sourceType;
			GUID idKey;
			HRESULT hr;

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			std::wstring link = converter.from_bytes(symlink);

			hr = MFCreateAttributes(&attributes, 2);
			THROW_IF_FAILED(hr, "MMF: Create attributes failed.");

			if (mediaType == MFMediaType_Audio) {
				sourceType = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID;
				idKey = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID;
			}
			else if (mediaType == MFMediaType_Video) {
				sourceType = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID;
				idKey = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK;
			}
			else {
				throw AVdevException("MMF: Media type is not supported.");
			}

			hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, sourceType);
			THROW_IF_FAILED(hr, "MMF: Set source type failed.");

			hr = attributes->SetString(idKey, link.c_str());
			THROW_IF_FAILED(hr, "MMF: Set source ID key failed.");

			hr = MFCreateDeviceSource(attributes, &mediaSource);
			THROW_IF_FAILED(hr, "MMF: Create video source failed.");

			*source = mediaSource;
			(*source)->AddRef();
		}

		void CreateTopology(GUID mediaType, IMFMediaSource * source, IMFActivate * sinkActivate, WCHAR * mmcssClass, IMFTopology ** topology)
		{
			jni::ComPtr<IMFTopology> pTopology;
			jni::ComPtr<IMFPresentationDescriptor> pd;
			jni::ComPtr<IMFStreamDescriptor> sd;
			jni::ComPtr<IMFMediaTypeHandler> handler;
			jni::ComPtr<IMFTopologyNode> srcNode;
			jni::ComPtr<IMFTopologyNode> dstNode;

			DWORD streams = 0;
			HRESULT hr;

			hr = MFCreateTopology(&pTopology);
			THROW_IF_FAILED(hr, "MMF: Create topology failed.");

			if (IsWindows7OrGreater()) {
				hr = pTopology->SetUINT32(MF_TOPOLOGY_HARDWARE_MODE, MFTOPOLOGY_HWMODE_USE_HARDWARE);
				THROW_IF_FAILED(hr, "MMF: Topology set hardware mode failed.");

				hr = pTopology->SetUINT32(MF_TOPOLOGY_DXVA_MODE, MFTOPOLOGY_DXVA_FULL);
				THROW_IF_FAILED(hr, "MMF: Topology set DXVA mode failed.");

				hr = pTopology->SetUINT32(MF_TOPOLOGY_ENUMERATE_SOURCE_TYPES, FALSE);
				THROW_IF_FAILED(hr, "MMF: Topology set enumerate source types failed.");
			}

			hr = source->CreatePresentationDescriptor(&pd);
			THROW_IF_FAILED(hr, "MMF: Create presentation descriptor failed.");

			hr = pd->GetStreamDescriptorCount(&streams);
			THROW_IF_FAILED(hr, "MMF: Get stream descriptor count failed.");

			for (DWORD i = 0; i < streams; i++) {
				BOOL selected = FALSE;
				GUID majorType;

				hr = pd->GetStreamDescriptorByIndex(i, &selected, &sd);
				THROW_IF_FAILED(hr, "MMF: Get stream descriptor by index failed.");

				hr = sd->GetMediaTypeHandler(&handler);
				THROW_IF_FAILED(hr, "MMF: Get media type handler failed.");

				hr = handler->GetMajorType(&majorType);
				THROW_IF_FAILED(hr, "MMF: Get major type failed.");

				if (majorType == mediaType && selected) {
					AddSourceNode(pTopology, source, pd, sd, mmcssClass, &srcNode);
					AddOutputNode(pTopology, sinkActivate, 0, &dstNode);

					hr = srcNode->ConnectOutput(0, dstNode, 0);
					THROW_IF_FAILED(hr, "MMF: Connect output node failed.");
					break;
				}
				else {
					hr = pd->DeselectStream(i);
					THROW_IF_FAILED(hr, "MMF: Deselect stream %d failed.", i);
				}
			}

			*topology = pTopology;
			(*topology)->AddRef();
		}

		void GetMediaTypeHandler(IMFMediaSource * source, DWORD streamIndex, IMFMediaTypeHandler ** typeHandler)
		{
			jni::ComPtr<IMFPresentationDescriptor> presDescriptor;
			jni::ComPtr<IMFStreamDescriptor> streamDescriptor;
			BOOL selected = FALSE;
			HRESULT hr;

			hr = source->CreatePresentationDescriptor(&presDescriptor);
			THROW_IF_FAILED(hr, "MMF: Create presentation descriptor failed.");

			hr = presDescriptor->GetStreamDescriptorByIndex(streamIndex, &selected, &streamDescriptor);
			THROW_IF_FAILED(hr, "MMF: Get stream descriptor by index failed.");

			hr = streamDescriptor->GetMediaTypeHandler(typeHandler);
			THROW_IF_FAILED(hr, "MMF: Get media type handler failed.");
		}

		template <class T>
		HRESULT GetTopologyNodeObject(IMFTopologyNode * node, T ** object)
		{
			IUnknown * unk = nullptr;
			HRESULT hr = node->GetObject(&unk);

			if (SUCCEEDED(hr)) {
				unk->QueryInterface(IID_PPV_ARGS(object));
				unk->Release();
			}
			return hr;
		}

		HRESULT BindOutputNode(IMFTopologyNode * node)
		{
			jni::ComPtr<IMFActivate> activate;
			jni::ComPtr<IMFMediaSink> mediaSink;
			jni::ComPtr<IMFStreamSink> streamSink;

			HRESULT hr;

			// The object pointer should be one of the following:
			// 1. An activation object for the media sink.
			// 2. The stream sink.

			// If it's #2, then we're already done.

			// First, check if it's an activation object.
			hr = GetTopologyNodeObject(node, &activate);

			if (SUCCEEDED(hr) && activate) {
				DWORD streamID = 0;

				// Try to create the media sink.
				hr = activate->ActivateObject(IID_PPV_ARGS(&mediaSink));

				// Look up the stream ID. (Default to zero.)
				if (SUCCEEDED(hr)) {
					streamID = MFGetAttributeUINT32(node, MF_TOPONODE_STREAMID, 0);
				}

				// Check if the media sink already has a stream sink with the requested ID.
				if (SUCCEEDED(hr)) {
					hr = mediaSink->GetStreamSinkById(streamID, &streamSink);
					
					if (FAILED(hr)) {
						// Try to add a new stream sink.
						hr = mediaSink->AddStreamSink(streamID, NULL, &streamSink);
					}
				}

				// Replace the node's object pointer with the stream sink. 
				if (SUCCEEDED(hr)) {
					hr = node->SetObject(streamSink);
				}
			}
			else {
				// Not an activation object. Is it a stream sink?
				hr = GetTopologyNodeObject(node, &streamSink);
			}

			return hr;
		}

		void BindOutputNodes(IMFTopology * topology)
		{
			jni::ComPtr<IMFCollection> outputCollection;
			jni::ComPtr<IUnknown> unk;
			jni::ComPtr<IMFTopologyNode> pNode;

			DWORD nodeCount = 0;

			HRESULT hr = topology->GetOutputNodeCollection(&outputCollection);
			THROW_IF_FAILED(hr, "MMF: Topology get output node collection failed.");

			hr = outputCollection->GetElementCount(&nodeCount);

			if (SUCCEEDED(hr)) {
				for (DWORD i = 0; i < nodeCount; i++) {
					hr = outputCollection->GetElement(i, &unk);

					if (FAILED(hr)) { break; }

					hr = unk->QueryInterface(IID_IMFTopologyNode, (void**)&pNode);

					if (FAILED(hr)) { break; }

					hr = BindOutputNode(pNode);

					if (FAILED(hr)) { break; }
				}
			}
		}

		void ListTopologyTransforms(IMFTopology * topology)
		{
			BindOutputNodes(topology);

			jni::ComPtr<IMFTopoLoader> topoLoader;
			jni::ComPtr<IMFTopology> resolvedTopology;
			jni::ComPtr<IMFTopologyNode> node;
			jni::ComPtr<IMFTransform> transform;
			jni::ComPtr<IMFMediaType> mediaType;
			WORD nodeCount = 0;
			HRESULT hr;

			hr = MFCreateTopoLoader(&topoLoader);
			THROW_IF_FAILED(hr, "MMF: Create topology loader failed.");

			hr = topoLoader->Load(topology, &resolvedTopology, nullptr);
			THROW_IF_FAILED(hr, "MMF: Resolve topology failed.");

			hr = resolvedTopology->GetNodeCount(&nodeCount);
			THROW_IF_FAILED(hr, "MMF: Topology get node count failed.");

			for (WORD i = 0; i < nodeCount; i++) {
				MF_TOPOLOGY_TYPE nodeType = MF_TOPOLOGY_MAX;

				hr = resolvedTopology->GetNode(i, &node);
				THROW_IF_FAILED(hr, "MMF: Topology get node failed.");

				hr = node->GetNodeType(&nodeType);
				THROW_IF_FAILED(hr, "MMF: Node get type failed.");

				if (nodeType != MF_TOPOLOGY_TRANSFORM_NODE) {
					continue;
				}

				hr = GetTopologyNodeObject(node, &transform);

				if (SUCCEEDED(hr)) {
					hr = transform->GetInputCurrentType(0, &mediaType);
					THROW_IF_FAILED(hr, "MMF: Transform get current input type failed.");

					printf("Transform Input\n");
					LogMediaType(mediaType);
					printf("\n");

					hr = transform->GetOutputCurrentType(0, &mediaType);
					THROW_IF_FAILED(hr, "MMF: Transform get current output type failed.");

					printf("Transform Output\n");
					LogMediaType(mediaType);
					printf("\n");
				}
			}

			fflush(NULL);
		}

		HRESULT QueueEventWithIUnknown(IMFMediaEventGenerator * gen, MediaEventType eventType, HRESULT status, IUnknown * unk)
		{
			PROPVARIANT var;
			var.vt = VT_UNKNOWN;
			var.punkVal = unk;

			unk->AddRef();

			HRESULT hr = gen->QueueEvent(eventType, GUID_NULL, status, &var);

			PropVariantClear(&var);

			return hr;
		}
	}
}