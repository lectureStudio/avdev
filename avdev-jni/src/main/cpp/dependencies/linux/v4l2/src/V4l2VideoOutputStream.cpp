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
#include "V4l2VideoOutputStream.h"
#include "V4l2TypeConverter.h"
#include "Log.h"

namespace avdev {

	V4l2VideoOutputStream::V4l2VideoOutputStream(std::string devDescriptor, PVideoSink sink) :
		VideoOutputStream(sink),
		maxBuffers(2),
		devDescriptor(devDescriptor)
	{
	}

	V4l2VideoOutputStream::~V4l2VideoOutputStream()
	{
		switch (getState()) {
			case StreamState::STARTED:
				stop();
				close();
				break;

			case StreamState::STOPPED:
			case StreamState::OPENED:
				close();
				break;

            default:
                break;
		}

		freeBuffer();
	}

	PictureFormat V4l2VideoOutputStream::getPictureFormat()
	{
		struct v4l2_format fmt = { 0 };
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_G_FMT, &fmt) == -1) {
			// May happen, if the device was initialised recently.

			PictureFormat format = VideoOutputStream::getPictureFormat();

			LOGDEV_WARN("V4l2: Failed to get picture format from %s.", devDescriptor.c_str());
			LOGDEV_WARN("V4l2: Returning default format: %s.", format.toString().c_str());

			return format;
		}

		struct v4l2_pix_format * pixformat = &fmt.fmt.pix;

		PixelFormat pixFormat = V4l2TypeConverter::toPixelFormat(pixformat->pixelformat);
		PictureFormat format(pixformat->width, pixformat->height, pixFormat);

		return format;
	}

	float V4l2VideoOutputStream::getFrameRate()
	{
		struct v4l2_streamparm parm = { 0 };
		parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_G_PARM, &parm) == -1) {
			return VideoOutputStream::getFrameRate();
		}

		struct v4l2_fract * tpf = &parm.parm.capture.timeperframe;
		float rate = 1.F * tpf->denominator / tpf->numerator;

		return rate;
	}

	void V4l2VideoOutputStream::openInternal()
	{
		PictureFormat format = VideoOutputStream::getPictureFormat();

		struct v4l2_pix_format * pixformat;
		struct v4l2_format fmt = { 0 };
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		pixformat = &fmt.fmt.pix;
		pixformat->width = format.getWidth();
		pixformat->height = format.getHeight();
		pixformat->pixelformat = V4l2TypeConverter::toApiType(format.getPixelFormat());
		pixformat->field = V4L2_FIELD_NONE;

		v4l2_fd = v4l2::openDevice(devDescriptor.c_str(), O_RDWR | O_NONBLOCK);

		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_S_FMT, &fmt) == -1) {
			throw AVdevException("V4l2: Failed setting picture format %dx%d %s.",
				pixformat->width, pixformat->height, ToFccString(pixformat->pixelformat).c_str());
		}

		struct v4l2_streamparm parm = { 0 };
		parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		struct v4l2_fract * tpf = &parm.parm.capture.timeperframe;
		tpf->numerator = 1;
		tpf->denominator = (int) VideoOutputStream::getFrameRate();

		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_S_PARM, &parm) == -1) {
			throw AVdevException("V4l2: Failed to set frame rate for %s.", devDescriptor.c_str());
		}

		float rate = tpf->denominator / tpf->numerator;
		printf("V4l2: Capturing rate: %.2f fps.\n", rate);
		printf("V4l2: Capturing format: %dx%d %s.\n",
			pixformat->width, pixformat->height,
			ToFccString(pixformat->pixelformat).c_str());

		/*
		 * Since video4linux adjusts the settings to its needs, if they are
		 * not supported, we need to adopt them.
		 */
		PixelFormat pixelFormat = V4l2TypeConverter::toPixelFormat(pixformat->pixelformat);
		PictureFormat outputFormat(pixformat->width, pixformat->height, pixelFormat);

		if (format != outputFormat) {
            LOGDEV_DEBUG("Format: User [%s] <> Device [%s]", format.toString().c_str(), outputFormat.toString().c_str());

            converter = std::make_shared<avdev::PixelFormatConverter>();
            converter->init(outputFormat, format);
		}

		setPictureFormat(outputFormat);

		initBuffer(pixformat->sizeimage * 2);
	}

	void V4l2VideoOutputStream::closeInternal()
	{
		switch (ioMethod) {
			case v4l2::IOMethod::READ:
				free(getBuffer(0));
				break;

			case v4l2::IOMethod::MMAP:
				for (unsigned i = 0; i < buffers.size(); ++i) {
					if (munmap(getBuffer(i), getBufferSize(i)) == -1) {
						printf("V4l2: Failed to unmap buffer %d.\n", i);
					}
				}
				break;

			case v4l2::IOMethod::USERPTR:
				for (unsigned i = 0; i < buffers.size(); ++i) {
					free(getBuffer(i));
				}
				break;
		}

		buffers.clear();
		buffers.shrink_to_fit();

		v4l2::closeDevice(v4l2_fd);
	}

	void V4l2VideoOutputStream::startInternal()
	{
		if (ioMethod == v4l2::IOMethod::MMAP) {
			for (unsigned i = 0; i < buffers.size(); ++i) {
				struct v4l2_buffer buf = { 0 };

				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index = i;

				if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_QBUF, &buf) == -1) {
					throw AVdevException("V4l2: Failed querying buffer %d for %s.", i, devDescriptor.c_str());
				}
			}

			enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_STREAMON, &type) == -1) {
				throw AVdevException("V4l2: Failed starting stream for %s.", devDescriptor.c_str());
			}
		}

		startThread();
	}

	void V4l2VideoOutputStream::stopInternal()
	{
		stopThreadAndWait();

		if (ioMethod != v4l2::IOMethod::READ) {
			enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			v4l2::ioctlDevice(v4l2_fd, VIDIOC_STREAMOFF, &type);
		}
	}

	void V4l2VideoOutputStream::run()
	{
		fd_set fds;
		struct timeval tv = {0};
		int r;

		while (isRunning()) {
			FD_ZERO(&fds);
			FD_SET(v4l2_fd, &fds);

			// Timeout
			tv.tv_sec = 1;
			tv.tv_usec = 0;

			r = select(v4l2_fd + 1, &fds, NULL, NULL, &tv);
			if (r < 0) {
				printf("V4l2: Select failed: %s.\n", strerror(errno));
				break;
			}

			if (r == 0) {
				printf("V4l2: Select timeout: %s.\n", strerror(errno));
				break;
			}

			if ((r > 0) && (FD_ISSET(v4l2_fd, &fds))) {
				if (captureFrame() == -1) {
					printf("V4l2: Failed capturing a frame from %s.\n", devDescriptor.c_str());
					break;
				}
			}
		}
	}

	int V4l2VideoOutputStream::captureFrame()
	{
		if (ioMethod == v4l2::IOMethod::READ) {
			if (read(v4l2_fd, getBuffer(0), getBufferSize(0)) == -1) {
				if (errno == EAGAIN)
					return 0;
				else
					return -1;
			}

			if (converter) {
                PictureFormat format = converter->getOutputFormat();
                unsigned width = format.getWidth();
                unsigned height = format.getHeight();
                unsigned size = width * height;

                converter->convert(getBuffer(0), buffer.data(), static_cast<int>(size));

                size_t frameSize = size * format.getBytesPerPixel();

                writeVideoFrame(buffer.data(), frameSize);
            }
            else {
                writeVideoFrame(getBuffer(0), getBufferSize(0));
            }
		}
		else if (ioMethod == v4l2::IOMethod::MMAP) {
			struct v4l2_buffer buf = { 0 };
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;

			if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_DQBUF, &buf) == -1) {
				if (errno == EAGAIN)
					return 0;
				else
					return -1;
			}

			/*
			std::uint8_t * output;
			size_t outputSize = buf.bytesused;

			try {
				jpegDecoder.transform(getBuffer(buf.index), buf.bytesused, &output, &outputSize);
			}
			catch (AVdevException & e) {
				printf("ERROR: %s \n", e.what());
			}

			writeVideoFrame(output, outputSize);
            */

			if (converter) {
                PictureFormat format = converter->getOutputFormat();
                unsigned width = format.getWidth();
                unsigned height = format.getHeight();
                unsigned size = width * height;

                converter->convert(getBuffer(buf.index), buffer.data(), static_cast<int>(size));

                size_t frameSize = size * format.getBytesPerPixel();

                writeVideoFrame(buffer.data(), frameSize);
            }
            else {
                writeVideoFrame(getBuffer(buf.index), buf.bytesused);
            }

			if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_QBUF, &buf) == -1) {
				printf("V4l2: Failed to enqueue buffer.\n");
			}
		}

		return 1;
	}

	void V4l2VideoOutputStream::initBuffer(unsigned int pictureSize)
	{
		struct v4l2_capability cap;

		if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_QUERYCAP, &cap) == -1) {
			throw AVdevException("V4l2: Failed to query device capabilities: %s.", devDescriptor.c_str());
		}

		if (cap.capabilities & V4L2_CAP_STREAMING) {
			struct v4l2_requestbuffers req = { 0 };
			req.count = maxBuffers;
			req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			req.memory = V4L2_MEMORY_MMAP;

			if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_REQBUFS, &req) == -1) {
				throw AVdevException("V4l2: Failed requesting buffer for %s.", devDescriptor.c_str());
			}

			buffers.resize(req.count);

			if (buffers.size() != req.count) {
				throw AVdevException("V4l2: Failed to allocate buffer memory for %s.", devDescriptor.c_str());
			}

			for (unsigned i = 0; i < req.count; ++i) {
				struct v4l2_buffer buf = { 0 };
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index = i;

				if (v4l2::ioctlDevice(v4l2_fd, VIDIOC_QUERYBUF, &buf) == -1) {
					throw AVdevException("V4l2: Failed querying buffer for %s.", devDescriptor.c_str());
				}

				void * data = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, v4l2_fd, buf.m.offset);

				buffers[i] = std::make_pair(data, buf.length);

				if (getBuffer(i) == MAP_FAILED) {
					throw AVdevException("V4l2: Failed to map buffer memory for %s.", devDescriptor.c_str());
				}
			}

			ioMethod = v4l2::IOMethod::MMAP;
		}
		else if (cap.capabilities & V4L2_CAP_READWRITE) {
			buffers.resize(1);

			if (buffers.empty()) {
				throw AVdevException("V4l2: Failed to create buffer for %s.", devDescriptor.c_str());
			}

			buffers[0] = std::make_pair(nullptr, pictureSize);
			buffers[0].first = malloc(pictureSize);

			if (!getBuffer(0)) {
				throw AVdevException("V4l2: Failed to allocate buffer memory for %s.", devDescriptor.c_str());
			}

			ioMethod = v4l2::IOMethod::READ;
		}
		else {
			throw AVdevException("V4l2: Failed to determine IO method for %s.", devDescriptor.c_str());
		}

		Stream::initBuffer(pictureSize * 2);
	}

	std::uint8_t * V4l2VideoOutputStream::getBuffer(std::uint8_t index)
	{
		return (std::uint8_t *) buffers[index].first;
	}

	size_t V4l2VideoOutputStream::getBufferSize(std::uint8_t index)
	{
		return buffers[index].second;
	}

}
