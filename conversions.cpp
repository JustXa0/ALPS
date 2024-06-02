#include <locale>
#include <codecvt>
#include "conversions.h"

std::string Conversions::wvector_to_string(std::vector<std::wstring>* wchar, int index) {
	std::wstring internal_message = wchar->at(index);
	std::string return_string;
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return_string = converter.to_bytes(internal_message);

	return return_string;
}

std::string Conversions::rectVector_to_string(std::vector<RECT>* rvector, int index, int flags) {
	switch (flags) {
		case 0:
		{
			RECT internal_rect = rvector->at(index);
			std::ostringstream oss;
			oss << "Right: " << internal_rect.right << ", Bottom: " << internal_rect.bottom;
			return oss.str();
		}
		case 1:
		{
			RECT internal_rect = rvector->at(index);
			std::ostringstream oss;
			oss << "Left: " << internal_rect.left << ", Top: " << internal_rect.top << ", Right: " << internal_rect.right << ", Bottom: " << internal_rect.bottom;
			return oss.str();
		}
		default:
		{
			RECT internal_rect = rvector->at(index);
			std::ostringstream oss;
			oss << "Left: " << internal_rect.left << ", Top: " << internal_rect.top << ", Right: " << internal_rect.right << ", Bottom: " << internal_rect.bottom;
			return oss.str();
		}
	}
}

Conversions::RectInts Conversions::rectVector_to_int(std::vector<RECT>* rvector, int index) {
	RectInts result;
	RECT internalRect = rvector->at(index);
	result.left = static_cast<int>(internalRect.left);
	result.top = static_cast<int>(internalRect.top);
	result.right = static_cast<int>(internalRect.right);
	result.bottom = static_cast<int>(internalRect.bottom);
	return result;
}

void Conversions::bytes_to_words(const std::vector<unsigned char>& bytes, std::vector<uint32_t>& words) {
	size_t numWords = bytes.size() / 4;
	words.resize(numWords);

	for (size_t i = 0; i < numWords; i++) {
		words[i] = 0;
		for (size_t j = 0; j < 4; j++) {
			words[i] |= static_cast<uint32_t>(bytes[i * 4 + j]) << (8 * j);
		}
	}
}