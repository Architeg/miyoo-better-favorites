// Host-only cJSON bridge for loader/parser observation; not target json-c timings.
// Production continues to link its existing ARM json-c library.
#ifndef BETTER_FAVORITES_PREVIEW_JSON_H
#define BETTER_FAVORITES_PREVIEW_JSON_H
#include <cJSON.h>
using json_object = cJSON;
enum json_type {json_type_object, json_type_string};
inline json_object* json_tokener_parse(const char* text){return cJSON_Parse(text);}
struct json_tokener { bool success=false; };
constexpr int JSON_TOKENER_STRICT=1;
enum json_tokener_error {json_tokener_success,json_tokener_error_parse};
inline json_tokener* json_tokener_new(){return new json_tokener;}
inline void json_tokener_set_flags(json_tokener*,int){}
inline json_object* json_tokener_parse_ex(json_tokener* parser,const char* text,int length){
    auto* result=cJSON_ParseWithLengthOpts(text,length,nullptr,1);parser->success=result!=nullptr;return result;
}
inline json_tokener_error json_tokener_get_error(json_tokener* parser){return parser->success?json_tokener_success:json_tokener_error_parse;}
inline void json_tokener_free(json_tokener* parser){delete parser;}
inline int json_object_object_get_ex(json_object* object,const char* key,json_object** value){*value=cJSON_GetObjectItemCaseSensitive(object,key);return *value!=nullptr;}
inline int json_object_is_type(json_object* object,json_type type){return type==json_type_object?cJSON_IsObject(object):cJSON_IsString(object);}
inline const char* json_object_get_string(json_object* object){return cJSON_GetStringValue(object);}
inline int json_object_get_int(json_object* object){return object?int(cJSON_GetNumberValue(object)):0;}
inline int json_object_get_boolean(json_object* object){return cJSON_IsTrue(object);}
inline void json_object_put(json_object* object){cJSON_Delete(object);}
#endif
