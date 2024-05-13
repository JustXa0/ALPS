#pragma once

#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#define FULL 1
#define DISPLAY 0

#include <string>
#include <sstream>
#include <vector>
#include <windows.h>

namespace Conversions {

	std::string wvector_to_string(std::vector<std::wstring>* wchar, int index);

	std::string rectVector_to_string(std::vector<RECT>* rvector, int index, int flags);

	void bytes_to_words(const std::vector<unsigned char>& bytes, std::vector<uint32_t>& words);

}



#endif