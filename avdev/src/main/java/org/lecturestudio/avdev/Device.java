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

public abstract class Device {

	private long nativeHandle;
	
	private String name;
	
	private String descriptor;
	
	
	public String getName() {
		return name;
	}
	
	public String getDescriptor() {
		return descriptor;
	}
	
	@Override
	public String toString() {
		return getName() + " (ID: " + getDescriptor() + ")";
	}
	
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((descriptor == null) ? 0 : descriptor.hashCode());
		result = prime * result + ((name == null) ? 0 : name.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (getClass() != obj.getClass()) {
			return false;
		}

		Device other = (Device) obj;

		if (nativeHandle == other.nativeHandle) {
			// Points to the same address.
			return true;
		}

		if (descriptor == null) {
			if (other.getDescriptor() != null) {
				return false;
			}
		}
		else if (!descriptor.equals(other.getDescriptor())) {
			return false;
		}

		if (name == null) {
			return other.getName() == null;
		}
		else {
			return name.equals(other.getName());
		}
	}
	
}
