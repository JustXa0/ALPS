#include "logger.h"

Logger::Log Logger::systemLogger("AlpsLog.txt");

/**
* \brief Constructor for Logging system, used internal to create global logger object
* 
* \param filepath - Location of file relative to run location, will be changed to be absolute in future update
*/
Logger::Log::Log(const std::string filepath) {
	Logger::Log::m_logfile.open(filepath);
	addLog(Logger::info, "Started Logging System.");
}

/**
* \brief Destructor for Logging system, automatically sends a closing message before destroying the object
*/
Logger::Log::~Log() {
	Logger::Log::addLog(Logger::info, "Stopped logging system.");
	Logger::Log::m_logfile.close();
}