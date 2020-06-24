#pragma once

#define WIN_ICONV_USING_STATIC
#include "convert/converter.hpp"

#include "frijson.hpp"

#include "../cebnf/cebnf.hpp"
#include "../cebnf/cebnf_helper.hpp"

namespace frijson {
	class Parser {
		friend class Json;
	public:
		Parser() {
			setCEBNF();
		}

		Json parse(const String& str) {

			std::unique_ptr<cebnf::SyntaxNode> syntax_tree;

			/*UTF-8 BOM*/
			if ((unsigned char)str[0] == 0xEF && (unsigned char)str[1] == 0xBB && (unsigned char)str[2] == 0xBF) {
				syntax_tree = _jbase.parse(wash(str.substr(3)));
			}
			else {
				syntax_tree = _jbase.parse(wash(str));
			}

			/*syntax error check*/
			if (!syntax_tree) return std::move(Json::createNull());

			return std::move(parseImpl_Base(syntax_tree));

		}

		Json parse(std::istream& is, const std::vector<const char*>& fromlist = { "UTF-8" , "SHIFFT_JIS-MS" }) {
			if (!is) {
				std::cerr << "Error: cannot load param json file" << std::endl;
				return std::move(Json::createNull());
			}

			std::string buf, tmp;
			while (std::getline(is, tmp)) {
				buf += tmp;
			}


			return std::move(parse(c3::Converter::auto_convert(buf, fromlist, "utf-8")));
		}

	private:
		enum JsonType {
			JSON_BASE,
			JSON_NULL,
			JSON_BOOL,
			JSON_BOOL_TRUE,
			JSON_BOOL_FALSE,
			JSON_STRING,
			JSON_NUMERIC,
			JSON_OBJECT,
			JSON_ARRAY,
		};

		cebnf::CEBNF<JSON_BASE		> _jbase;
		cebnf::CEBNF<JSON_NULL		> _jnull;
		cebnf::CEBNF<JSON_BOOL		> _jbool;
		cebnf::CEBNF<JSON_STRING	> _jstring;
		cebnf::CEBNF<JSON_NUMERIC	> _jnumeric;
		cebnf::CEBNF<JSON_OBJECT	> _jobject;
		cebnf::CEBNF<JSON_ARRAY		> _jarray;

		void setCEBNF() {
			using namespace cebnf;
			CEBNF_OperatorTools t;

			_jnull      = Term("null");
			_jbool      = Term("true", JSON_BOOL_TRUE) | Term("false", JSON_BOOL_FALSE);
			_jstring    = StringIE2('"', '"');
			_jnumeric   = (Integer() | RealNumber()) - t[(Term("E") | Term("e")) - Integer()];

			_jobject    = Term("{") - t[_jstring - Term(":") - _jbase - t({ Term(",") - _jstring - Term(":") - _jbase })] - Term("}");
			_jarray     = Term("[") - t[_jbase - t({ Term(",") - _jbase })] - Term("]");

			_jbase		= _jnull | _jbool | _jstring | _jnumeric | _jobject | _jarray;
		}


		Json parseImpl_Base(std::unique_ptr<cebnf::SyntaxNode>& node) {
			switch (node->children[0]->getTokenID()) {
			case JSON_NULL:
				return std::move(parseImpl_Null(node->children[0]));
			case JSON_BOOL:
				return std::move(parseImpl_Bool(node->children[0]));
			case JSON_STRING:
				return std::move(parseImpl_String(node->children[0]));
			case JSON_NUMERIC:
				return std::move(parseImpl_Numeric(node->children[0]));
			case JSON_OBJECT:
				return std::move(parseImpl_Object(node->children[0]));
			case JSON_ARRAY:
				return std::move(parseImpl_Array(node->children[0]));
			default:
				return std::move(Json::createNull());
			}
		}

		Json parseImpl_Null(std::unique_ptr<cebnf::SyntaxNode>& node) {
			return std::move(Json::createNull());
		}

		Json parseImpl_Bool(std::unique_ptr<cebnf::SyntaxNode>& node) {
			if (node->children[0]->getTokenID() == JSON_BOOL_TRUE) {
				return std::move(Json::createBool(true));
			}
			else {
				return std::move(Json::createBool(false));
			}

		}

		/*JsonString -> StringIE2 -> {<">, InnerString, <">}*/
		String getString_ofSyntaxTreeJsonString(std::unique_ptr<cebnf::SyntaxNode>& node) {
			if (node->children[0]->children.size() == 3) {
				return std::move(node->children[0]->children[1]->getString());
			}
			else if (node->children[0]->children.size() == 1) {
				return std::move(node->children[0]->children[0]->getString());
			}
		}

		Json parseImpl_String(std::unique_ptr<cebnf::SyntaxNode>& node) {
			return std::move(Json::createString(getString_ofSyntaxTreeJsonString(node)));
		}

		Json parseImpl_Numeric(std::unique_ptr<cebnf::SyntaxNode>& node) {
			return std::move(Json::createNumeric(node->getString()));
		}

		Json parseImpl_Object(std::unique_ptr<cebnf::SyntaxNode>& node) {
			const size_t children_size = node->children.size();

			Json obj = Json::createObject();
			/*first: { */
			size_t itr = 1;
			/*obj children size:: 2, 5, 9, pre+4...*/
			while (itr + 1 < children_size) {
				obj._dat->getObj()[getString_ofSyntaxTreeJsonString(node->children[itr])] = parseImpl_Base(node->children[itr + 2]);
				/*add itr:: {String, <:>, JsonData} and comma<,>*/
				itr += 4;
			}

			return std::move(obj);
		}

		Json parseImpl_Array(std::unique_ptr<cebnf::SyntaxNode>& node) {
			const size_t children_size = node->children.size();
			const size_t arr_size = (children_size - 1) / 2;

			Json arr = Json::createArray(arr_size);
			/*first: [ */
			size_t itr = 1;
			size_t arr_itr = 0;
			/*arr children size:: 2, 3, 5, pre+2...*/
			while (itr + 1 < children_size) {
				arr._dat->getArr().at(arr_itr) = parseImpl_Base(node->children[itr]);
				/*add {JsonData} and comma<,>*/
				itr += 2;
				arr_itr++;
			}

			return std::move(arr);
		}

		/*remove spaces, tabs, and newlines without them in strings("...")*/
		String wash(const String& str) {
			String dst;
			dst.reserve(str.length());
			size_t itr = 0;
			bool string_flag = false;
			while (str[itr] != '\0') {
				if (str[itr] == '"') {
					string_flag = !string_flag;
					dst += '"';
				}
				else if (str[itr] == '\\') {
					dst += '\\';
					itr++;
					dst += str[itr];
				}
				else if (str[itr] == ' ' || str[itr] == '\t' || str[itr] == '\r' || str[itr] == '\n') {
					if (string_flag) {
						dst += str[itr];
					}
				}
				else {
					dst += str[itr];
				}

				itr++;
			}

			return std::move(dst);
		}


	};

}
