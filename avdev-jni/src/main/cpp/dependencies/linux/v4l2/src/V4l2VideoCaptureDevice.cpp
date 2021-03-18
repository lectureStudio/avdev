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

#include "AVdevException.h"
#include "V4l2VideoCaptureDevice.h"
#include "V4l2VideoOutputStream.h"
#include "V4l2TypeConverter.h"

namespace avdev
{
	V4l2VideoCaptureDevice::V4l2VideoCaptureDevice(std::string name, std::string descriptor)
		: VideoCaptureDevice(name, descriptor)
	{
		v4l2_fd = v4l2::openDevice(getDescriptor().c_str(), O_RDONLY);
		if (v4l2_fd < 0) {
			throw AVdevException("V4l2: Failed to open device: %s.", getName().c_str());
		}
	}

	V4l2VideoCaptureDevice::~V4l2VideoCaptureDevice()
	{
		v4l2::closeDevice(v4l2_fd);
	}

	std::list<PictureFormat> V4l2VideoCaptureDevice::getPictureFormats()
	{
		if (formats.empty()) {
			struct v4l2_capability vcap = { 0 };

			if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_QUERYCAP, &vcap) == -1) {
				throw AVdevException("V4l2: Failed to query device: %s.", getName().c_str());
			}

			struct v4l2_fmtdesc fmt = { 0 };
			struct v4l2_frmsizeenum frameSize = { 0 };

			fmt.index = 0;
			fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			
			while (v4l2::ioctlDevice(v4l2_fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {
				PixelFormat pixFormat = V4l2TypeConverter::toPixelFormat(fmt.pixelformat);

				frameSize.pixel_format = fmt.pixelformat;
				frameSize.index = 0;
				
				while (v4l2::ioctlDevice(v4l2_fd, VIDIOC_ENUM_FRAMESIZES, &frameSize) > -1) {
					if (frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
						PictureFormat format(frameSize.discrete.width, frameSize.discrete.height, pixFormat);
						formats.push_back(format);
					}
					else if (frameSize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
						PictureFormat format(frameSize.stepwise.max_width, frameSize.stepwise.max_height, pixFormat);
						formats.push_back(format);
					}
					frameSize.index++;
				}
				fmt.index++;
			}
		}
		
		return formats;
	}
	
	std::list<PictureControl> V4l2VideoCaptureDevice::getPictureControls()
	{
		std::list<PictureControl> controls;

		struct v4l2_queryctrl queryctrl = { 0 };

		for (queryctrl.id = V4L2_CID_USER_BASE; queryctrl.id < V4L2_CID_LASTP1; queryctrl.id++) {
			if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_QUERYCTRL, &queryctrl) == 0) {
				if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
					continue;
				}
				
				//printf("%s %d \n", queryctrl.name, queryctrl.id);

				PictureControlType type;
				bool autoMode = false;
				bool changed = false;

				switch (queryctrl.id) {
					case V4L2_CID_AUTOGAIN:
						type = PictureControlType::Gain;
						autoMode = true;
						break;
					case V4L2_CID_HUE_AUTO:
						type = PictureControlType::Hue;
						autoMode = true;
						break;
					case V4L2_CID_AUTO_WHITE_BALANCE:
						type = PictureControlType::WhiteBalance;
						autoMode = true;
						break;
						
					default:
						type = V4l2TypeConverter::toPictureControlType(queryctrl.id);
				}

				for (std::list<PictureControl>::iterator itr = controls.begin(); itr != controls.end();) {
					if ((*itr).getType() == type) {
						// Merge controls of same type into one, e.g. V4L2_CID_HUE and V4L2_CID_HUE_AUTO.
						PictureControl ctrl(PictureControlType::NotImplemented, 0, 0, 0, 0, false);

						if (!(*itr).hasAutoMode()) {
							ctrl = PictureControl(type, (*itr).getMinValue(), (*itr).getMaxValue(),
								(*itr).getStepValue(), (*itr).getDefaultValue(), true);
						}
						else {
							ctrl = PictureControl(type, queryctrl.minimum, queryctrl.maximum,
								queryctrl.step, queryctrl.default_value, true);
						}

						// Replace control.
						itr = controls.erase(itr);
						controls.push_back(ctrl);

						changed = true;
						break;
					}
					++itr;
				}

				if (!changed) {
					PictureControl ctrl = PictureControl(type, queryctrl.minimum, queryctrl.maximum,
						queryctrl.step, queryctrl.default_value, autoMode);

					controls.push_back(ctrl);
				}
			}
			else {
				if (errno == EINVAL) {
					continue;
				}

				throw AVdevException("V4l2: Query camera control failed: %s.", getName().c_str());
			}
		}
		
		return controls;
	}

	std::list<CameraControl> V4l2VideoCaptureDevice::getCameraControls()
	{
		std::list<CameraControl> controls;
		
		struct v4l2_queryctrl queryctrl = { 0 };
		
		queryctrl.id = V4L2_CTRL_CLASS_CAMERA | V4L2_CTRL_FLAG_NEXT_CTRL;
		
		while (v4l2::ioctlDevice(v4l2_fd, VIDIOC_QUERYCTRL, &queryctrl) == 0) {
			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
				continue;
			}
			if (queryctrl.id == V4L2_CID_JPEG_CLASS) {
				queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
				continue;
			}

			//printf("%s %x \n", queryctrl.name, queryctrl.id);

			CameraControlType type;
			bool autoMode = false;
			bool changed = false;

			switch (queryctrl.id) {
				case V4L2_CID_EXPOSURE_AUTO:
					type = CameraControlType::Exposure;
					autoMode = true;
					break;
				case V4L2_CID_FOCUS_AUTO:
					type = CameraControlType::Focus;
					autoMode = true;
					break;
					
				default:
					type = V4l2TypeConverter::toCameraControlType(queryctrl.id);
			}

			for (std::list<CameraControl>::iterator itr = controls.begin(); itr != controls.end();) {
				if ((*itr).getType() == type) {
					// Merge controls of same type into one, e.g. V4L2_CID_FOCUS_* and V4L2_CID_FOCUS_AUTO.
					CameraControl ctrl(CameraControlType::NotImplemented, 0, 0, 0, 0, false);

					if (!(*itr).hasAutoMode()) {
						ctrl = CameraControl(type, (*itr).getMinValue(), (*itr).getMaxValue(),
							(*itr).getStepValue(), (*itr).getDefaultValue(), true);
					}
					else {
						ctrl = CameraControl(type, queryctrl.minimum, queryctrl.maximum,
							queryctrl.step, queryctrl.default_value, true);
					}

					// Replace control.
					itr = controls.erase(itr);
					controls.push_back(ctrl);

					changed = true;
					break;
				}
				++itr;
			}

			if (!changed) {
				CameraControl ctrl = CameraControl(type, queryctrl.minimum, queryctrl.maximum,
					queryctrl.step, queryctrl.default_value, autoMode);

				controls.push_back(ctrl);
			}
			
			queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
		}
		
		return controls;
	}

	void V4l2VideoCaptureDevice::setPictureControlAutoMode(PictureControlType type, bool autoMode)
	{
		struct v4l2_control control = { 0 };

		switch (type) {
			case PictureControlType::Gain:
				control.id = V4L2_CID_AUTOGAIN;
				break;
			case PictureControlType::Hue:
				control.id = V4L2_CID_HUE_AUTO;
				break;
			case PictureControlType::WhiteBalance:
				control.id = V4L2_CID_AUTO_WHITE_BALANCE;
				break;

			default:
				return;
		}

		control.value = (autoMode ? 1 : 0);

		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_S_CTRL, &control) != 0) {
			throw AVdevException("V4l2: Set picture control value failed: %d, value: %d.", type, autoMode);
		}
	}
	
	bool V4l2VideoCaptureDevice::getPictureControlAutoMode(PictureControlType type)
	{
		struct v4l2_control control = { 0 };
		
		switch (type) {
			case PictureControlType::Gain:
				control.id = V4L2_CID_AUTOGAIN;
				break;
			case PictureControlType::Hue:
				control.id = V4L2_CID_HUE_AUTO;
				break;
			case PictureControlType::WhiteBalance:
				control.id = V4L2_CID_AUTO_WHITE_BALANCE;
				break;

			default:
				throw AVdevException("V4l2: No auto-mode for specified picture control available.");
		}
		
		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_G_CTRL, &control) == 0) {
			return static_cast<bool>(control.value);
		}
		else if (errno != EINVAL) {
			throw AVdevException("V4l2: Get picture control failed. Control is not supported.");
		}
		
		throw AVdevException("V4l2: Get picture control failed: %d.", type);
	}

	void V4l2VideoCaptureDevice::setPictureControlValue(PictureControlType type, long value)
	{
		struct v4l2_queryctrl queryctrl = { 0 };
		
		queryctrl.id = V4l2TypeConverter::toApiType(type);
		
		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_QUERYCTRL, &queryctrl) == 0) {
			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
				return;
			}
			
			struct v4l2_control control = { 0 };
			control.id = queryctrl.id;
			control.value = std::max(queryctrl.minimum, std::min(queryctrl.maximum, static_cast<int>(value)));
			
			if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_S_CTRL, &control) != 0) {
				throw AVdevException("V4l2: Set picture control failed: %s, value: %ld.", queryctrl.name, value);
			}
		}
		else if (errno != EINVAL) {
			throw AVdevException("V4l2: Set picture control failed. Control is not supported.");
		}
	}
	
	long V4l2VideoCaptureDevice::getPictureControlValue(PictureControlType type)
	{
		struct v4l2_control control = { 0 };
		
		control.id = V4l2TypeConverter::toApiType(type);

		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_G_CTRL, &control) == 0) {
			return static_cast<long>(control.value);
		}
		else if (errno != EINVAL) {
			throw AVdevException("V4l2: Get picture control failed. Control is not supported.");
		}
		
		throw AVdevException("V4l2: Get picture control failed: %d.", type);
	}

	void V4l2VideoCaptureDevice::setCameraControlAutoMode(CameraControlType type, bool autoMode)
	{
		struct v4l2_control control = { 0 };

		switch (type) {
			case CameraControlType::Exposure:
				control.id = V4L2_CID_EXPOSURE_AUTO;
				break;
			case CameraControlType::Focus:
				control.id = V4L2_CID_FOCUS_AUTO;
				break;

			default:
				return;
		}

		control.value = (autoMode ? 1 : 0);

		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_S_CTRL, &control) != 0) {
			throw AVdevException("V4l2: Set camera control value failed: %d, value: %d.", type, autoMode);
		}
	}
	
	bool V4l2VideoCaptureDevice::getCameraControlAutoMode(CameraControlType type)
	{
		struct v4l2_control control = { 0 };
		
		switch (type) {
			case CameraControlType::Exposure:
				control.id = V4L2_CID_EXPOSURE_AUTO;
				break;
			case CameraControlType::Focus:
				control.id = V4L2_CID_FOCUS_AUTO;
				break;

			default:
				throw AVdevException("V4l2: No auto-mode for specified camera control available.");
		}
		
		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_G_CTRL, &control) == 0) {
			return static_cast<bool>(control.value);
		}
		else if (errno != EINVAL) {
			throw AVdevException("V4l2: Get camera control failed. Control is not supported.");
		}
		
		throw AVdevException("V4l2: Get camera control failed: %d.", type);
	}

	void V4l2VideoCaptureDevice::setCameraControlValue(CameraControlType type, long value)
	{
		struct v4l2_queryctrl queryctrl = { 0 };
		
		queryctrl.id = V4l2TypeConverter::toApiType(type);
		
		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_QUERYCTRL, &queryctrl) == 0) {
			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
				return;
			}
			
			struct v4l2_control control = { 0 };
			control.id = queryctrl.id;
			control.value = std::max(queryctrl.minimum, std::min(queryctrl.maximum, static_cast<int>(value)));
			
			if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_S_CTRL, &control) != 0) {
				throw AVdevException("V4l2: Set camera control failed: %s, value: %ld.", queryctrl.name, value);
			}
		}
		else if (errno != EINVAL) {
			throw AVdevException("V4l2: Set camera control failed. Control is not supported.");
		}
	}
	
	long V4l2VideoCaptureDevice::getCameraControlValue(CameraControlType type)
	{
		struct v4l2_control control = { 0 };
		control.id = V4l2TypeConverter::toApiType(type);
		
		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_G_CTRL, &control) == 0) {
			return static_cast<long>(control.value);
		}
		else if (errno != EINVAL) {
			throw AVdevException("V4l2: Get camera control failed. Control is not supported.");
		}
		
		throw AVdevException("V4l2: Get camera control failed: %d.", type);
	}
	
	void V4l2VideoCaptureDevice::setPictureFormat(PictureFormat format)
	{
		this->format = format;
	}
	
	PictureFormat const& V4l2VideoCaptureDevice::getPictureFormat() const
	{
		return format;
	}

	void V4l2VideoCaptureDevice::setFrameRate(float frameRate)
	{
		this->frameRate = frameRate;
	}
	
	float V4l2VideoCaptureDevice::getFrameRate() const
	{
		return frameRate;
	}
	
	PVideoOutputStream V4l2VideoCaptureDevice::createOutputStream(PVideoSink sink)
	{
		PVideoOutputStream stream = std::make_unique<V4l2VideoOutputStream>(getDescriptor(), sink);
		stream->setFrameRate(getFrameRate());
		stream->setPictureFormat(getPictureFormat());
		
		return stream;
	}

}
