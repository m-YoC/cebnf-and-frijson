#pragma once

#include <string>
#include <vector>
#include <type_traits>

#if _WIN32
#ifdef WIN_ICONV_USING_STATIC
#ifdef _WIN64
#include "winiconv/x64static/include/iconv.h"
#pragma comment(lib, "convert/winiconv/x64static/lib/win_iconv_x64_static.lib")
#else
#include "winiconv/x86static/include/iconv.h"
#pragma comment(lib, "convert/winiconv/x86static/lib/win_iconv_x86_static.lib")
#endif
#else
#ifdef _WIN64
#include "winiconv/x64/include/iconv.h"
#pragma comment(lib, "convert/winiconv/x64/lib/win_iconv_x64.lib")
#else
#include "winiconv/x86/include/iconv.h"
#pragma comment(lib, "convert/winiconv/x86/lib/win_iconv_x86.lib")
#endif
#endif
#elif _unix
#include <iconv.h>
#else
#include <iconv.h>
#endif

/*character code conversion*/
namespace c3 {

	class Converter {
	public:
		static std::string convert(const std::string& str, const char* from, const char* to) {

			iconv_t icd;

			size_t length = str.length();
			size_t dstlen = length * 2;
			if (length <= 0) return std::string("");

			char* srcstr = new char[length + 1];
			char* dststr = new char[dstlen + 1];

			strcpy_s(srcstr, length + 1, str.c_str());

			if ((icd = iconv_open(to, from)) == (iconv_t)-1) {
				delete[] srcstr;
				delete[] dststr;
				return "convert iconv_open error <from: " + std::string(from) + ", to: " + std::string(to) + ">";
			}

			const char* src_pos = srcstr;
			char* dst_pos = dststr;
			if (iconv(icd, &src_pos, &length, &dst_pos, &dstlen) == -1) {
				iconv_close(icd);
				delete[] srcstr;
				delete[] dststr;
				return "convert iconv error <from: " + std::string(from) + ", to: " + std::string(to) + ">";
			}

			*dst_pos = '\0';
			iconv_close(icd);

			std::string d(dststr);
			delete[] srcstr;
			delete[] dststr;

			return d;
		}

		static std::string auto_convert(const std::string& str, const std::vector<const char*>& fromlist, const char* to) {

			std::string dst;
			for (auto itr = fromlist.begin(); itr != fromlist.end(); ++itr) {

				if (convert_check(str, dst, *itr, to)) {
					//std::cout << "convert from " + std::string(*itr) << std::endl;
					return dst;
				}
			}

			return "convert fromlist error <to: " + std::string(to) + ">";
		}


	private:

		static bool convert_check(const std::string& str, std::string& dst_str, const char* from, const char* to) {

			iconv_t icd;

			size_t length = str.length();
			size_t dstlen = length * 2;
			if (length <= 0) {
				dst_str = '\0';
				return true;
			}

			char* srcstr = new char[length + 1];
			char* dststr = new char[dstlen + 1];

			strcpy_s(srcstr, length + 1, str.c_str());

			if ((icd = iconv_open(to, from)) == (iconv_t)-1) {
				delete[] srcstr;
				delete[] dststr;
				return false;
			}

			const char* src_pos = srcstr;
			char* dst_pos = dststr;
			if (iconv(icd, &src_pos, &length, &dst_pos, &dstlen) == -1) {
				iconv_close(icd);
				delete[] srcstr;
				delete[] dststr;
				return false;
			}
			*dst_pos = '\0';
			iconv_close(icd);

			dst_str = dststr;

			delete[] srcstr;
			delete[] dststr;

			return true;
		}

	};

}