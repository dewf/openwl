#include "unicodestuff.h"

#include <unicode/unistr.h>
#include <boost/locale/encoding_utf.hpp>
//#include <boost/locale.hpp>

using boost::locale::conv::utf_to_utf;

std::wstring utf8_to_wstring(const std::string &str) {
	return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}

std::string wstring_to_utf8(const std::wstring &str) {
	return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
}

// see here what is required for this to work: https://stackoverflow.com/questions/26990412/c-boost-crashes-while-using-locale
//std::string upperCased(std::string input) {
//    return boost::locale::to_upper(input);
//}

std::string upperCased(std::string input) {
	std::string output;
	auto uni = icu::UnicodeString::fromUTF8(input);
	auto upped = uni.toUpper();
	upped.toUTF8String(output);
	return output;
}

