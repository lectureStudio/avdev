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

public interface Logger {

	/**
     * Logs a message object with the {@code DEBUG} level.
     *
     * @param message the message string to log.
     */
    void debug(String message);
    
    /**
     * Logs a message object with the {@code ERROR} level.
     *
     * @param message the message string to log.
     */
    void error(String message);
    
    /**
     * Logs a message object with the {@code FATAL} level.
     *
     * @param message the message string to log.
     */
    void fatal(String message);
    
    /**
     * Logs a message object with the {@code INFO} level.
     *
     * @param message the message string to log.
     */
    void info(String message);
    
    /**
     * Logs a message object with the {@code WARN} level.
     *
     * @param message the message string to log.
     */
    void warn(String message);
	
}
