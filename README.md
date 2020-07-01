# CEBNF: Extended Backus-Naur Form on C++
 
 (beta version)  

 <---Exchange--->  
"text" -> Term("text")  
concatenation: comma( , ) -> hyphen( - ) (not exception symbol)  
optional: [ cebnf_expr ] -> t[ cebnf_expr ] with CEBNF_OperatorTools t;  
repetition: { cebnf_expr } -> t( cebnf_expr ) with CEBNF_OperatorTools t;  
or  { cebnf_expr } -> t({ cebnf_expr }) with CEBNF_OperatorTools t;  

<---note--->  
Cebnf does not implemnet exception symbol.  

If string including repetition and last string are same, for example ( { A | B | C }, A ), cebnf cannot parse it. Please create helper.  

Alternation( | ) should be NAND(at least one is false). When both are true, then cebnf adopts one with a longer acquisition string.  
For example, if the string "abcd..." matches both ("abc" ... and "ab" ...), then cebnf adopts "abc".  


## example code

This code is a part of frijson parser

```cpp
enum JsonType {
    JSON_BASE, JSON_NULL, JSON_BOOL, JSON_BOOL_TRUE, JSON_BOOL_FALSE,
    JSON_STRING, JSON_NUMERIC, JSON_OBJECT, JSON_ARRAY,
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

    _jbase      = _jnull | _jbool | _jstring | _jnumeric | _jobject | _jarray;
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

    /* and other parseImpl functions. */
```

and how to create cebnf syntax tree is as follows.

```cpp
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
```

## frijson

frijson is a sample code of json class and parser using cebnf.  
How to use is very simple and the parser can be easily modified.

Note that this code does not work.


```cpp
/*
frijson usage - example code

Character encode using frijson is utf-8 (or ascii).
*/

#include <iostream>
#include <fstream>

#include <unordered_map>

#include "frijson/frijson.hpp"
#include "frijson/frijson_p.hpp"

int main() {

    /* load json file with std::ifsteram */
    std::ifstream ifs("parameter.json");
    /* initialize frijson::Parser */
    frijson::Parser psr;


    /* parse json file (second arg is the character encoding list of json file. from list to utf-8...) */
    auto json = psr.parse(ifs, { "UTF-8", "SHIFT_JIS-MS" });
    /* or set std::string(utf-8) directly */
    // auto json = psr.parse(json_string);
    
    /* get integer from json object */
    int res_int = json["base"]["int_data"].numeric<int>();
    /* get float from json array */
    float res_float = json["base2"][2].numeric<float>();

    /*get bool and std::string(utf-8) */
    bool res_bool = json["base"]["bool_data"].boolean();
    auto res_str = json["str_data"][0].str();

    /* check json type */
    bool res_jsontype = json.isType(frijson::Json::eString); /* or json.isString(); */
    
    /* find key from json object */
    bool res_find = json.find("str_data");

    /* set data to object */
    json["new_key_1"] = "new_data";
    json["new_key_2"]["new_key_3"] = 3.14;
    /* 
        If type is eObject or eUndefined, add key (and create object) automatically.
        Otherwise error.

        safe type: json.at("new_key"); This function does not add key to object automatically.
        const type is also safe.
    */

    /* get array size */
    size_t size = json.size();

    /* set data to array */
    json["base2"][0] = "new_data";
    json["base2"][1][2] = 3.14;
    /*
        If type is eArray or eUndefined, expand size (and create array) automatically. Similar to object.
        Otherwise error.

        safe type: json.at(size_t i); This function does not expand size to array automatically.
        const type is also safe.
    */

    /* 
        std type and initializer_list can also be assigned to frijson. 

        ---to object---
        std::map<std::string, bool>; std::map<std::string, numeric_type>; std::map<std::string, std::string>;
        std::unordered_map<std::string, bool>; etc...

        ---to array---
        std::vector<bool>; std::vector<std::string>; std::array<numeric_type>; etc...
        {"data1", "data2", "data3"};
    */
    std::unordered_map<frijson::String, frijson::String> map;
    map["umap_key"] = "umap_data";
    json["new_key_umap"] = map;

    json["new_key_initializer_list"] = {"English", "Japanese", "Spanish"};

    return 0;
}
```

