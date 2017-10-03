
#ifndef __RAPIDJSONSAX_H__
#define __RAPIDJSONSAX_H__

#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <rapidjson/reader.h>
#include <rapidjson/filereadstream.h>

using namespace std;
using namespace rapidjson;

class CRapidJsonSAX
{
	private:
		size_t fileStreamBufSize;
		typedef void callbackParserFunc_t(int, string, int, CRapidJsonSAX*);

		struct parseHandler {
			int type;
			string data;

			parseHandler() : type(), data() {}

			bool Null() {
				type = type_Null;
				data.clear();
				return true;
			}
			bool Bool(bool b) {
				type = type_Bool;
				data = b ? "true" : "false";
				return true;
			}
			bool Int(int i) {
				type = type_Int;
				data = to_string(i);
				return true;
			}
			bool Uint(unsigned u) {
				type = type_Uint;
				data = to_string(u);
				return true;
			}
			bool Int64(int64_t i) {
				type = type_Int64;
				data = to_string(i);
				return true;
			}
			bool Uint64(uint64_t u) {
				type = type_Uint64;
				data = to_string(u);
				return true;
			}
			bool Double(double d) {
				type = type_Double;
				data = to_string(d);
				return true;
			}
			bool RawNumber(const char* str, SizeType length, bool) {
				type = type_Number;
				data = string(str, length);
				return true;
			}
			bool String(const char* str, SizeType length, bool) {
				type = type_String;
				data = string(str, length);
				return true;
			}
			bool StartObject() {
				type = type_StartObject;
				data.clear();
				return true;
			}
			bool Key(const char* str, SizeType length, bool) {
				type = type_Key;
				data = string(str, length);
				return true;
			}
			bool EndObject(SizeType memberCount) {
				type = type_EndObject;
				data = to_string(memberCount);
				return true;
			}
			bool StartArray() {
				type = type_StartArray;
				data.clear();
				return true;
			}
			bool EndArray(SizeType elementCount) {
				type = type_EndArray;
				data = to_string(elementCount);
				return true;
			}

			private:
				parseHandler(const parseHandler& noCopyConstruction);
				parseHandler& operator=(const parseHandler& noAssignment);
		};

		void Init();
		void parseStreamInternal(void* stream, bool isFileStream, callbackParserFunc_t* callback);

	public:
		enum {
			parser_Start,
			parser_Work,
			parser_Stop
		};
		enum {
			type_Null,
			type_Bool,
			type_Int,
			type_Uint,
			type_Int64,
			type_Uint64,
			type_Double,
			type_Number,
			type_String,
			type_Key,
			type_StartObject,
			type_EndObject,
			type_StartArray,
			type_EndArray,
			type_None
		};

		CRapidJsonSAX();
		~CRapidJsonSAX();

		string getTypeStr(int type);
		void parseFile(string file, callbackParserFunc_t* callback);
		void parseString(string json, callbackParserFunc_t* callback);
		void setFileStreamBufSize(size_t size) {
			fileStreamBufSize = size;
		}
};

#endif // __RAPIDJSONSAX_H__
