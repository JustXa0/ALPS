#pragma once

#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <fstream>
#include <string>
#include <mutex>
#include <ctime>

// As of right now this is only being developed for Windows devices so Win32 API functions will be used.
// In the future as the project is expanded this choice may be challenged and/or changed.
#include <shlobj.h>

namespace Logger {
	
	// Default levels of warnings used, more can be added if needed
	typedef enum level_t {
		info, debug, warning, error, fatal
	} level;

	class Log {
	public:
		/**
		* \brief Constructor for Logging system, used internally to create global logger object
		* 
		* \param filepath - Location of file relative to run location, will be changed to be absolute in future update
		*/
		Log(const std::wstring filepath);

		/**
		* \brief Destructor for Logging system, automatically sends a closing message before destroying the object
		*/
		~Log();

		/**
		* \brief Method used to add a message to the log, including a severity level
		* 
		* \param s - one of 5 level_t enumerated keywords used to resolve error level
		* \param msg - message to be written into log
		*/
		template <typename T>
		void addLog(level s, T msg) {
			std::lock_guard<std::mutex> lock(logMutex);
			if (m_logfile.is_open()) {
				m_logfile << levels[static_cast<int>(s)] << ": " << msg << std::endl;
			}
		}

	private:
		// Used to store the file input location
		std::ofstream m_logfile;

		// Array of strings to convert logging levels into writable version, must be the same order as the enum types
		std::string levels[5] = { "Info", "Debug", "Warning", "Error", "Fatal" };

		// Used to keep logger thread-safe
		std::mutex logMutex;

		// Used to store location of log file
		std::unique_ptr<Log> log_ptr;
	};

	// Global variable used for default logging
	extern Log systemLogger;
}

#endif