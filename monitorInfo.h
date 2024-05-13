#pragma once
#include <vector>
#include <string>
#include <wtypes.h>


namespace Monitor {

	class monitorInfo {
	public:

		/**
		* \brief Structure to store friendly names, display areas, and work areas of devices
		*/
		typedef struct monitor_t {
			std::vector<std::wstring> friendlyName;
			std::vector<RECT> displayArea;
			std::vector<RECT> workArea;
		} monitor;

		/**
		* \brief Constructor used to initialize a monitor struct
		* 
		* \param s - a pointer to a monitor struct
		*/
		monitorInfo(monitor* s);

		~monitorInfo();

	private:

		/**
		* \brief Internal method to find the friendly names of all connected monitors
		* 
		* \param friendlyName - a pointer to a vector of wide strings used for storing all friendly names
		* 
		* \return True if all friendly names are found, False if an error occurs during this process
		*/
		bool RetrieveMonitorFriendlyName(std::vector<std::wstring>* friendlyName);

		/**
		* \brief Internal method used to find the display area of all connected monitors
		* 
		* \param displayArea - a pointer to a vector of RECT used for storing all display areas
		* 
		* \return True if all display areas are found, False if an error occurs during this process
		*/
		bool RetrieveMonitorDisplayArea(std::vector<RECT>* displayArea);

		/**
		* \brief Internal method used to find the work area of all connected monitor
		* 
		* \param workArea - a pointer to a vector of RECT used for storing all work areas
		* 
		* \return True if all work areas are found, False if an error occurs during this process
		*/
		bool RetrieveMonitorWorkArea(std::vector<RECT>* workArea);

	};
};

