#ifndef CPP_LUA_PARSER_LUASCRIPT_H_
#define CPP_LUA_PARSER_LUASCRIPT_H_

#include <string>
#include <vector>
#include <iostream>
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include "Utilities.hpp"

class LuaScript {
public:
	bool isScriptLoaded = false;
	std::string name = "";

	LuaScript(const std::string& scriptName);
	~LuaScript();

	template<typename T> T Get(const std::string& variableName);
	template<typename T> std::vector<T> GetVector(const std::string& arrayName);
	void RunScript();

private:
	lua_State* L;
	int currentLevel;

	void OutputError(const std::string& variableName, const std::string& error);
	void Clean();
	template<typename T> T GetDefaultValue();
	template<typename T> T GetValueFromLua(const std::string& variableName);
	bool HandleLuaStack(const std::string& variableName);
};

// This function handles getting variables from the LuaScript.
template<typename T> T LuaScript::Get(const std::string& variableName) {
	// Check the script is loaded and accessible.
	if (!L) {
		OutputError(variableName, "Script was not loaded, .Get() failed");
		return GetDefaultValue<T>();
	}

	T luaValue;	// Stores the value from the lua script.
	if (HandleLuaStack(variableName)) {
		luaValue = GetValueFromLua<T>(variableName);
	}
	else {
		luaValue = GetDefaultValue<T>();
	}

	Clean();
	return luaValue;
}
template<typename T> inline std::vector<T> LuaScript::GetVector(const std::string & arrayName) {
	std::vector<T> arrayFromLua;

	if (!L) {
		OutputError(arrayName, "Script was not loaded, .GetVector() failed");
		return std::vector<T>();
	}

	HandleLuaStack(arrayName.c_str());

	// Check we can find the array.
	if (lua_isnil(L, -1)) {
		return std::vector<T>();
	}

	lua_pushnil(L);
	while (lua_next(L, -2)) {
		arrayFromLua.push_back(GetValueFromLua<T>(arrayName));
		lua_pop(L, 1);
	}
	Clean();
	return arrayFromLua;
}
// Functions returning a value when things go wrong.
template<typename T> inline T LuaScript::GetDefaultValue() {
	return 0;
}
template<> inline std::string LuaScript::GetDefaultValue<std::string>() {
	return "null";
}
// Functions handling the conversion from the lua state into the values we want to use.
template<typename T> T LuaScript::GetValueFromLua(const std::string & variableName) {
	return 0;
}
template<> inline int LuaScript::GetValueFromLua(const std::string & variableName) {
	if (!lua_isnumber(L, -1)) {
		OutputError(variableName, "Not a number");
	}
	return (int)lua_tonumber(L, -1);
}
template<> inline float LuaScript::GetValueFromLua(const std::string& variableName) {
	if (!lua_isnumber(L, -1)) {
		OutputError(variableName, "Not a number");
	}
	return (float)lua_tonumber(L, -1);
}
template<> inline bool LuaScript::GetValueFromLua(const std::string& variableName) {
	return (bool)lua_toboolean(L, -1);
}
template<> inline std::string LuaScript::GetValueFromLua(const std::string& variableName) {
	if (!lua_isstring(L, -1)) {
		OutputError(variableName, "Not a string");
	}
	return (std::string)lua_tostring(L, -1);
}
#endif