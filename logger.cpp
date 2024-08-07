#include "logger.h"

Logger::Log Logger::systemLogger(L"AlpsLog");

/**
* \brief Internal function used to get the local time of the computer
*/
tm* getTime() {
	time_t rawtime;
	tm* timeInfo;
	time(&rawtime);
	timeInfo = localtime(&rawtime);
	return timeInfo;
}

/**
* \brief Constructor for Logging system, used internally to create global logger object
* 
* \param filepath - Name of the file to be created, the date and time will be appended.
*/
Logger::Log::Log(const std::wstring filepath) {
	PWSTR roaming = NULL;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &roaming);
	if (SUCCEEDED(hr)) {
		CreateDirectory((std::wstring(roaming) + L"\\ALPS\\").c_str(), NULL);
		tm* timeInfo = getTime();
		std::wstring subfolder = std::wstring(roaming) + L"\\ALPS\\" 
			+ filepath + L" " + std::to_wstring(timeInfo->tm_mon + 1) 
			+ L"_" + std::to_wstring(timeInfo->tm_mday) + L"_" 
			+ std::to_wstring(timeInfo->tm_year + 1900) + L"_" + std::to_wstring(timeInfo->tm_hour) 
			+ L"_" + std::to_wstring(timeInfo->tm_min) + L"_" + std::to_wstring(timeInfo->tm_sec) 
			+ L".txt";
		Logger::Log::m_logfile.open(subfolder);
		CoTaskMemFree(roaming);
	}
	else {
		Logger::Log::m_logfile.open(filepath);
	}addLog(Logger::info, "Started Logging System.");
}

/**
* \brief Destructor for Logging system, automatically sends a closing message before destroying the object
*/
Logger::Log::~Log() {
	Logger::Log::addLog(Logger::info, "Stopped logging system.");
	Logger::Log::m_logfile.close();
}

