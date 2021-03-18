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

import java.lang.System.Logger.Level;

public class SystemLogger implements Logger {

	private final System.Logger logger;


	public SystemLogger(System.Logger logger) {
		this.logger = logger;
	}

	@Override
	public void debug(String message) {
		logger.log(Level.DEBUG, message);
	}

	@Override
	public void error(String message) {
		logger.log(Level.ERROR, message);
	}

	@Override
	public void fatal(String message) {
		logger.log(Level.ERROR, message);
	}

	@Override
	public void info(String message) {
		logger.log(Level.INFO, message);
	}

	@Override
	public void warn(String message) {
		logger.log(Level.WARNING, message);
	}
}
