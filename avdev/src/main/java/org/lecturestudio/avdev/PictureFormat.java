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

package org.lecturestudio.avdev;

import java.util.HashMap;
import java.util.Map;

public class PictureFormat {

	public static int FourCC(String fccString) {
		int value = 0;
		value |= (fccString.charAt(0) << 0);
		value |= (fccString.charAt(1) << 8);
		value |= (fccString.charAt(2) << 16);
		value |= (fccString.charAt(3) << 24);

		return value;
	}

	public static String FourCC(int fccValue) {
		String fccString = "";
		fccString += (char) ((fccValue >> 0)  & 0xFF);
		fccString += (char) ((fccValue >> 8)  & 0xFF);
		fccString += (char) ((fccValue >> 16) & 0xFF);
		fccString += (char) ((fccValue >> 24) & 0xFF);
		return fccString;
	}

	public enum PixelFormat {

		/**  8  RGB-3-3-2      */		RGB332	(FourCC("RGB8")),
		/** 16  RGB-5-5-5      */		RGB555	(FourCC("R555")),
		/** 16  RGB-5-6-5      */		RGB565	(FourCC("R565")),
		/** 24  BGR-8-8-8      */		BGR24	(FourCC("BGR3")),
		/** 24  RGB-8-8-8      */		RGB24	(FourCC("RGB3")),
		/** 32  BGR-8-8-8-8    */		BGR32	(FourCC("BGR4")),
		/** 32  RGB-8-8-8-8    */		RGB32	(FourCC("RGB4")),
		/**  8  Greyscale      */		GREY	(FourCC("GREY")),
		/**  8  4:2:2 Packed   */		YUY2    (FourCC("YUY2")),
		/**  8  4:2:0 Planar   */		YV12    (FourCC("YV12")),
		/**  9  YVU 4:1:0      */		YVU410	(FourCC("YVU1")),
		/**  9  YUV 4:1:0      */		YUV410	(FourCC("YUV1")),
		/** 12  YVU 4:2:0      */		YVU420	(FourCC("YVU2")),
		/** 12  YUV 4:2:0      */		I420	(FourCC("I420")),
		/** 16  YUV 4:2:2      */		YUYV	(FourCC("YUYV")),
		/** 16  YUV 4:2:2      */		UYVY	(FourCC("UYVY")),
		/** 16  YVU 422 planar */		YUV422P	(FourCC("I422")),
		/** 16  YVU 411 planar */		YUV411P	(FourCC("I411")),
		/** 12  YUV 4:1:1      */		Y41P	(FourCC("Y41P")),
		/** 12  Y/CbCr 4:2:0   */		NV12	(FourCC("NV12")),
		/** 12  Y/CrCb 4:2:0   */		NV21	(FourCC("NV21")),
		/**  8  8-bit color    */		HI240	(FourCC("HI24")),
		/** JFIF JPEG          */		JPEG	(FourCC("JPEG")),
		/** MPEG               */		MPEG	(FourCC("MPEG")),
		/** Motion-JPEG        */		MJPG	(FourCC("MJPG")),
		/** 1394               */		DV		(FourCC("DV  ")),
		/** Winnov hw compress */		WNVA	(FourCC("WNVA")),
										UNKNOWN	(FourCC("UNKN"));

		static final Map<Integer, PixelFormat> MAP = new HashMap<>();

		static {
			for (PixelFormat format : PixelFormat.values()) {
				MAP.put(format.id, format);
			}
		}

		int id;


		PixelFormat(int id) {
			this.id = id;
		}

		public int getId() {
			return id;
		}

		public static PixelFormat byId(int id) {
			return MAP.get(id);
		}
	}



	private final PixelFormat format;

	private final int width;

	private final int height;


	public PictureFormat(PixelFormat format, int width, int height) {
		this.format = format;
		this.width = width;
		this.height = height;
	}

	public PixelFormat getPixelFormat() {
		return format;
	}

	public int getWidth() {
		return width;
	}

	public int getHeight() {
		return height;
	}

	@Override
	public String toString() {
		return FourCC(format.getId()) + " - " + width + "x" + height;
	}

}
