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

public abstract class VideoControl<T> {

	private final T type;
	private final long min;
	private final long max;
	private final long step;
	private final long def;
	private final boolean autoMode;

	
	abstract public String getName();
	
	
	public VideoControl(T type, long min, long max, long step, long def, boolean autoMode) {
		this.type = type;
		this.min = min;
		this.max = max;
		this.step = step;
		this.def = def;
		this.autoMode = autoMode;
	}
	
	public T getType() {
		return type;
	}
	
	public long getMinValue() {
		return min;
	}
	
	public long getMaxValue() {
		return max;
	}
	
	public long getStepValue() {
		return step;
	}
	
	public long getDefaultValue() {
		return def;
	}
	
	public boolean hasAutoMode() {
		return autoMode;
	}
	
	@Override
	public String toString() {
		return getName() + ": " + min + ":" + step + ":" + max + ", Default Value: " + def + ", Auto-Mode: " + autoMode;
	}
	
}
