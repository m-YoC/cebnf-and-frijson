#pragma once

#include "frijson.hpp"

namespace frijson {

	class Dumper {
	public:
		static String Dump(Json& json) {
			return DumpValue(json);
		}

		/*add utf-8 byte order mark <EF BB BF>*/
		static String AddBom(String& str) {
			String bom;
			bom += 0xEF;
			bom += 0xBB;
			bom += 0xBF;
			return bom + str;
		}

	private:
		static String StringDumpConvert(const String& str) {
			size_t f = 0;
			frijson::String dst;
			dst += '\"';

			while (str[f] != '\0') {

				if (str[f] == '\\') {
					dst += "\\\\";
					f++;
					continue;
				}
				if (str[f] == '\"') {
					dst += "\\\"";
					f++;
					continue;
				}
				if (str[f] == '/') {
					dst += "\\/";
					f++;
					continue;
				}
				if (str[f] == '\b') {
					dst += "\\b";
					f++;
					continue;
				}
				if (str[f] == '\f') {
					dst += "\\f";
					f++;
					continue;
				}
				if (str[f] == '\n') {
					dst += "\\n";
					f++;
					continue;
				}
				if (str[f] == '\r') {
					dst += "\\r";
					f++;
					continue;
				}
				if (str[f] == '\t') {
					dst += "\\t";
					f++;
					continue;
				}

				dst += str[f];
				f++;

			}

			dst += '\"';
			return dst;
		}
		static String DumpValue(Json& json) {

			if (json.isNull()) {
				return u8"null";
			}
			if (json.isBool()) {
				if (json.boolean()) {
					return u8"true";
				}
				else {
					return u8"false";
				}
			}

			if (json.isString()) {
				return StringDumpConvert(json.str());
			}

			if (json.isNumeric()) {
				return json.str();
			}

			if (json.isObject()) {
				frijson::String dst;
				dst += '{';
				if (json._dat->getObj().begin() == json._dat->getObj().end()) {
					return "";
				}
				for (auto itr = json._dat->getObj().begin(); itr != json._dat->getObj().end(); itr++) {

					dst += StringDumpConvert(itr->first);
					dst += ':';
					dst += DumpValue(itr->second);
					dst += ',';
				}
				dst.back() = '}';

				return dst;
			}

			if (json.isArray()) {
				frijson::String dst;
				dst += '[';
				if (json._dat->getArr().begin() == json._dat->getArr().end()) {
					return "";
				}
				for (auto itr = json._dat->getArr().begin(); itr != json._dat->getArr().end(); itr++) {
					dst += DumpValue(*itr);
					dst += ',';
				}
				dst.back() = ']';

				return dst;
			}

			return u8"null";
		}
	};



}
