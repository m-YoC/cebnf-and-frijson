#pragma once


/*
簡易jsonパーサ frijson

内部ではUTF8で保存しています．
C++11以降 u8"xxxx"でUTF8形式の文字列を扱えます．
ASCII限定ならu8修飾はいらないと思います．
JPN Windows環境だと標準出力等がShift-JIS(MS):CP932だと思われるので，
日本語等を利用する場合はUTF8形式に変換してください．
Shift-JIS環境だと一部の2バイト文字に"\"や"|"といったASCII文字コードが含まれていることがあるので(ソや表など一般的な文字もある)
バグります．


パーサ本体はfrijson_p.hppの方にあります．
自分でjsonファイルをstd::string型に読み込んでぶち込んでください．
パーサ本体は自分で書いたお遊びコードを使ってるのでそんなに速くないと思います(他のパーサと未比較)．
	

できるだけ簡便に書けるようにしていますが，その分若干特殊な挙動を示す場合があります．
[]演算子でObjectやArrayのkey/index指定をした場合，勝手にkeyが追加されたりarrayが増築されたりします．
Jsonクラスに何も入っていない場合(frijson::Json::ValueType::eUndefined(eNullとは別))，[]演算子でkey/index指定をしたとき勝手に対応した型になります．
これは
json["undefined_key1"]["undefined_key2"]["undefined_key3"] = "string";
みたいな感じで自分でobjectやarrayを構成する際に楽に書けるようにするためのものです．

これはatメソッドを使った場合は発生しません．
また，const指定をした場合も発生しません．


*/

#include <string>
#include <map>
#include <vector>
#include <type_traits>
#include <memory>
#include <assert.h>
#include <iostream>

namespace frijson {

	class Json;

	class JsonBase;
	class JsonNull;
	class JsonBool;
	class JsonString;
	class JsonNumber;
	class JsonObject;
	class JsonArray;

	using String = std::string;
	using Object = std::map<frijson::String, frijson::Json>;
	using Array = std::vector<frijson::Json>;

	static String FRIJSON_NULL_STRING = u8"";
	static Object FRIJSON_NULL_OBJECT = Object();
	static Array FRIJSON_NULL_ARRAY = Array();

	class JsonBase {
	public:
		virtual ~JsonBase() {}

		virtual bool getBool() const { return false; }

		virtual String& getStr() { return FRIJSON_NULL_STRING; }
		virtual Object& getObj() { return FRIJSON_NULL_OBJECT; }
		virtual Array&  getArr() { return FRIJSON_NULL_ARRAY; }

		virtual const String& getStr() const { return FRIJSON_NULL_STRING; }
		virtual const Object& getObj() const { return FRIJSON_NULL_OBJECT; }
		virtual const Array&  getArr() const { return FRIJSON_NULL_ARRAY; }
	};

	class JsonNull : public JsonBase {
	public:
		//virtual ~JsonNull() {}
	};

	class JsonBool : public JsonBase {
	public:
		//virtual ~JsonBool() {}

		JsonBool(bool e) : value(e){}
		virtual bool getBool() const override { return value; }

	private:
		bool value;
	};

	class JsonString : public JsonBase {
	public:
		//virtual ~JsonString() {}

		JsonString(String& e) : value(e) {}
		JsonString(const String& e) : value(e) {}
		virtual String& getStr() override { return value; }
		virtual const String& getStr() const override { return value; }

	private:
		frijson::String value;
	};

	class JsonNumber : public JsonBase {
	public:
		//virtual ~JsonNumber() {}

		JsonNumber(String& e) : value(e) {}
		JsonNumber(const String& e) : value(e) {}
		virtual String& getStr() override { return value; }
		virtual const String& getStr() const override { return value; }

	private:
		String value;
	};

	class JsonObject : public JsonBase {
	public:
		//virtual ~JsonObject() {}

		JsonObject(Object& e) : value(e) {}
		JsonObject(const Object& e) : value(e) {}
		virtual Object& getObj() override { return value; }
		virtual const Object& getObj() const override { return value; }

	private:
		frijson::Object value;
	};

	class JsonArray : public JsonBase {
	public:
		//virtual ~JsonArray() {}

		JsonArray(Array& e) : value(e) {}
		JsonArray(const Array& e) : value(e) {}
		virtual Array& getArr() override { return value; }
		virtual const Array& getArr() const override { return value; }

	private:
		Array value;
	};

	

	class Json {
		friend class Json;
		friend class Parser;
		friend class Dumper;
	public:
		enum ValueType {
			eUndefined, eNull, eBool, eString, eNumeric, eObject, eArray
		};

		Json() = default;
		Json(ValueType t, JsonBase* json_base) : _type(t), _dat(json_base) {}

		void SetJson(ValueType t, JsonBase* json_base) {
			_type = t;
			_dat.reset(json_base);
		}

		bool isNull()		const { return _type == ValueType::eNull;	}
		bool isBool()		const { return _type == ValueType::eBool;	}
		bool isString()		const { return _type == ValueType::eString;	}
		bool isNumeric()	const { return _type == ValueType::eNumeric;	}
		bool isObject()		const { return _type == ValueType::eObject;	}
		bool isArray()		const { return _type == ValueType::eArray;	}

		bool isType(ValueType e) const { return _type == e; }

		bool boolean() const { return _dat->getBool(); }

		String& str() { return _dat->getStr(); }
		const String& str() const { return _dat->getStr(); }

		template<class NumericType>
		NumericType numeric() const;

		template<> char					numeric<char				>() const { return (char)std::stoi(_dat->getStr());			}
		template<> unsigned char		numeric<unsigned char		>() const { return (unsigned char)std::stoi(_dat->getStr());	}
		template<> float				numeric<float				>() const { return std::stof(_dat->getStr());				}
		template<> double				numeric<double				>() const { return std::stod(_dat->getStr());				}
		template<> long double			numeric<long double			>() const { return std::stold(_dat->getStr());				}
		template<> int					numeric<int					>() const { return std::stoi(_dat->getStr());				}
		template<> unsigned int			numeric<unsigned int		>() const { return (unsigned int)std::stoul(_dat->getStr());	}
		template<> long					numeric<long				>() const { return std::stol(_dat->getStr());				}
		template<> long long			numeric<long long			>() const { return std::stoll(_dat->getStr());				}
		template<> unsigned long		numeric<unsigned long		>() const { return std::stoul(_dat->getStr());				}
		template<> unsigned long long	numeric<unsigned long long	>() const { return std::stoull(_dat->getStr());				}
		
		/*find object key*/
		bool find(const String& key) const {
			if (_dat->getObj().count(key) == 0) {
				return false;
			}
			else {
				return true;
			}
		}

		/*unsafe: Available new key*/
		Json& operator[](const String& key) {
			if (this->_type == ValueType::eObject) {
				return _dat->getObj()[key];
			}
			else if(this->_type == Json::ValueType::eUndefined){
				*this = createObject();
				return _dat->getObj()[key];
			}
			else {
				assert(false);
			}
		}

		/*safe: only existing key*/
		const Json& operator[](const String& key) const {
			if (!find(key)) {
				assert(false);
			}
			return _dat->getObj().at(key);
		}

		/*safe: only existing key*/
		Json& at(const String& key) {
			if (!find(key)) {
				assert(false);
			}
			return _dat->getObj().at(key);
		}

		/*safe: only existing key*/
		const Json& at(const String& key) const {
			if (!find(key)) {
				assert(false);
			}
			return _dat->getObj().at(key);
		}

		/*get array size*/
		size_t size() {
			return _dat->getArr().size();
		}

		/*unsafe: variable size*/
		Json& operator[](size_t idx) {
			if (this->_type == ValueType::eArray) {
				if (idx >= _dat->getArr().size()) {
					_dat->getArr().resize(idx + 1);
				}
				return _dat->getArr().at(idx);
			}
			else if (this->_type == Json::ValueType::eUndefined) {
				*this = createArray(idx + 1);
				return _dat->getArr().at(idx);
			}
			else {
				assert(false);
			}
		}

		/*safe: const size*/
		const Json& operator[](size_t idx) const {
			if (idx >= _dat->getArr().size()) {
				assert(false);
			}
			return _dat->getArr().at(idx);
		}

		/*safe: const size*/
		Json& at(size_t idx) {
			if (idx >= _dat->getArr().size()) {
				assert(false);
			}
			return _dat->getArr().at(idx);
		}

		/*safe: const size*/
		const Json& at(size_t idx) const {
			if (idx >= _dat->getArr().size()) {
				assert(false);
			}
			return _dat->getArr().at(idx);
		}




		Json& operator=(bool b) {
			return (*this = createBool(b));
		}

		Json& operator=(const String& str) {
			return (*this = createString(str));
		}

		Json& operator=(const char* str) {
			return (*this = createString(str));
		}

		template<class NumericType, typename std::enable_if<std::is_scalar<NumericType>::value>::type* = nullptr>
		Json& operator=(NumericType num) {
			return (*this = createNumeric(num));
		}


		template<class MapType,
			typename std::enable_if< std::is_same<frijson::String, typename MapType::key_type>::value && 
			(
				std::is_same<bool, typename MapType::mapped_type>::value ||
				std::is_same<frijson::String, typename MapType::mapped_type>::value ||
				std::is_scalar<typename MapType::mapped_type>::value
			)
			>::type* = nullptr>
		Json& operator=(const MapType& m) {
			return (*this = createObject(m));
		}

		template<class VecType,
			typename std::enable_if<
			std::is_same<bool, typename VecType::value_type>::value || 
			std::is_same<frijson::String, typename VecType::value_type>::value || 
			std::is_scalar<typename VecType::value_type>::value
			>::type* = nullptr>
		Json& operator=(const VecType& v) {
			return (*this = createArray(v));
		}

		template<class type,
			typename std::enable_if<
			std::is_same<bool, type>::value ||
			std::is_same<frijson::String, type>::value ||
			std::is_scalar<type>::value
			>::type* = nullptr>
		Json& operator=(std::initializer_list<type> v) {
			return (*this = createArray(std::move(v)));
		}




		static Json createNull() {
			return Json(Json::ValueType::eNull, new JsonNull());
		}

		static Json createBool(bool b) {
			return Json(Json::ValueType::eBool, new JsonBool(b));
		}

		static Json createString(const String& str) {
			return Json(Json::ValueType::eString, new JsonString(str));
		}
		static Json createString(const char* str) {
			return Json(Json::ValueType::eString, new JsonString(String(str)));
		}

		template<class NumericType, typename std::enable_if<std::is_scalar<NumericType>::value>::type* = nullptr>
		static Json createNumeric(NumericType num) {
			return Json(Json::ValueType::eNumeric, new JsonNumber(std::to_string(num)));
		}

		static Json createNumeric(const String& num) {
			return Json(Json::ValueType::eNumeric, new JsonNumber(num));
		}
		static Json createNumeric(const char* num) {
			return Json(Json::ValueType::eNumeric, new JsonNumber(String(num)));
		}


		static Json createObject() {
			return Json(Json::ValueType::eObject, new JsonObject(Object()));
		}

		template<class MapType,
			typename std::enable_if< std::is_same<String, typename MapType::key_type>::value
			&& std::is_same<bool, typename MapType::mapped_type>::value
		>::type* = nullptr>
			static Json createObject(const MapType& m) {
			auto json = Json(Json::ValueType::eObject, new JsonObject(Object()));

			for (auto itr = m.begin(); itr != m.end(); itr++) {
				json._dat->getObj()[itr->first] = createBool(itr->second);
			}

			return json;
		}

		template<class MapType, 
			typename std::enable_if< std::is_same<String, typename MapType::key_type>::value
			&& std::is_same<String, typename MapType::mapped_type>::value
			>::type* = nullptr>
		static Json createObject(const MapType& m) {
			auto json = Json(Json::ValueType::eObject, new JsonObject(Object()));

			for (auto itr = m.begin();itr != m.end(); itr++) {
				json._dat->getObj()[itr->first] = createString(itr->second);
			}		

			return json;
		}

		template<class MapType,
			typename std::enable_if< std::is_same<String, typename MapType::key_type>::value
			&& std::is_scalar<typename MapType::mapped_type>::value
		>::type* = nullptr>
			static Json createObject(const MapType& m) {
			auto json = Json(Json::ValueType::eObject, new JsonObject(Object()));

			for (auto itr = m.begin(); itr != m.end(); itr++) {
				json._dat->getObj()[itr->first] = createNumeric(itr->second);
			}

			return json;
		}

		static Json createArray() {
			return Json(Json::ValueType::eArray, new JsonArray(Array()));
		}

		static Json createArray(size_t size) {
			return Json(Json::ValueType::eArray, new JsonArray(Array(size)));
		}

		template<class VecType,
			typename std::enable_if<std::is_same<bool, typename VecType::value_type>::value>::type* = nullptr>
		static Json createArray(const VecType& v) {
			auto json = Json(Json::ValueType::eArray, new JsonArray(Array(v.size())));

			size_t i = 0;
			for (auto itr = v.begin(); itr != v.end(); itr++) {
				json._dat->getArr()[i++] = createBool(*itr);
			}

			return json;
		}

		template<class type,
			typename std::enable_if<std::is_same<bool, type>::value>::type* = nullptr>
			static Json createArray(std::initializer_list<type> v) {
			auto json = Json(Json::ValueType::eArray, new JsonArray(Array(v.size())));

			size_t i = 0;
			for (auto itr = v.begin(); itr != v.end(); itr++) {
				json._dat->getArr()[i++] = createBool(*itr);
			}

			return json;
		}

		template<class VecType,
			typename std::enable_if<std::is_same<String, typename VecType::value_type>::value>::type* = nullptr>
			static Json createArray(const VecType& v) {
			auto json = Json(Json::ValueType::eArray, new JsonArray(Array(v.size())));

			size_t i = 0;
			for (auto itr = v.begin(); itr != v.end(); itr++) {
				json._dat->getArr()[i++] = createString(*itr);
			}

			return json;
		}

		template<class type,
			typename std::enable_if<std::is_same<String, type>::value>::type* = nullptr>
			static Json createArray(std::initializer_list<type> v) {
			auto json = Json(Json::ValueType::eArray, new JsonArray(Array(v.size())));

			size_t i = 0;
			for (auto itr = v.begin(); itr != v.end(); itr++) {
				json._dat->getArr()[i++] = createString(*itr);
			}

			return json;
		}

		template<class VecType,
			typename std::enable_if<std::is_scalar<typename VecType::value_type>::value>::type* = nullptr>
			static Json createArray(const VecType& v) {
			auto json = Json(Json::ValueType::eArray, new JsonArray(Array(v.size())));

			size_t i = 0;
			for (auto itr = v.begin(); itr != v.end(); itr++) {
				json._dat->getArr()[i++] = createNumeric(*itr);
			}

			return json;
		}

		template<class type,
			typename std::enable_if<std::is_scalar<type>::value>::type* = nullptr>
			static Json createArray(std::initializer_list<type> v) {
			auto json = Json(Json::ValueType::eArray, new JsonArray(Array(v.size())));

			size_t i = 0;
			for (auto itr = v.begin(); itr != v.end(); itr++) {
				json._dat->getArr()[i++] = createNumeric(*itr);
			}

			return json;
		}

	private:

		ValueType _type = Json::ValueType::eUndefined;
		std::shared_ptr<JsonBase> _dat = nullptr;

	};

}
