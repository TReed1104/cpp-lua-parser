#ifndef CPP_LUA_PARSER_LUASCRIPT_HPP_
#define CPP_LUA_PARSER_LUASCRIPT_HPP_

#include <string>
#include <vector>
#include <iostream>
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

class LuaScript {
public:
	bool isScriptLoaded = false;
	std::string name = "";

	LuaScript(const std::string& scriptName) {
		name = scriptName;
		L = luaL_newstate();
		// Makes sure the file is loaded.
		if (luaL_loadfile(L, scriptName.c_str()) || lua_pcall(L, 0, 0, 0)) {
			std::cout << "Error: script not loaded (" << scriptName << ")" << std::endl;
			L = 0;
		}
		if (L) {
			isScriptLoaded = true;
			luaL_openlibs(L);
		}
	}
	~LuaScript() {
		if (L) {
			lua_close(L);
		}
	}

	template<typename T> T Get(const std::string& variableName) {
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
	template<typename T> std::vector<T> GetVector(const std::string& arrayName) {
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

private:
	lua_State* L;
	int currentLevel;

	// General Functions
	std::vector<std::string> stringSplit(const std::string& stringToSplit, const char& splitToken) {
		// Splits a string using the given splitToken, E.g. ""The.Cat.Sat.On.The.Mat" splits with token '.' into Vector[6] = {The, Cat, Sat, On, The, Mat};

		std::vector<std::string> splitString;	// Stores the split sections of string for the return.
		std::string currentSplit = "";			// Stores the current section being split off.

		size_t sizeOfStringArray = stringToSplit.size();			// .Size() of a const so it never changes and we store it once.
		for (size_t i = 0; i < sizeOfStringArray; i++) {
			char currentChar = stringToSplit.at(i);
			if (currentChar == splitToken) {
				splitString.push_back(currentSplit);
				currentSplit = "";
			}
			else {
				currentSplit += currentChar;
			}

			if (i == sizeOfStringArray - 1 && currentChar != splitToken) {
				// Catches the final section of string as there might not be a follow up split token.
				splitString.push_back(currentSplit);
			}
		}

		return splitString;
	}
	void Clean() {
		// Clean up the stack
		int n = lua_gettop(L);
		lua_pop(L, n);
	}
	void OutputError(const std::string& variableName, const std::string& error) {
		std::cout << "Error: Unable to get " << variableName << " due to error: " << error << std::endl;
	}
	bool HandleLuaStack(const std::string& variableName) {
		// Split the variableName into its hierarchy.
		std::vector<std::string> variableNameSplit = stringSplit(variableName, '.');
		const size_t numberOfLevels = variableNameSplit.size();	// Store the size and .size() recalculates the size each call and the size is constant here.
		for (size_t i = 0; i < numberOfLevels; i++) {
			if (i == 0) {
				// If we are at the head of the stack, get global.
				lua_getglobal(L, variableNameSplit[i].c_str());
			}
			else {
				// If we are not at the head, get the accessible field with the current level of the stack.
				lua_getfield(L, -1, variableNameSplit[i].c_str());
			}
			if (lua_isnil(L, -1)) {
				// Not found, kick out an error and return false from the function.
				OutputError(variableName, i + " is not defined");
				return false;
			}
		}
		return true;
	}

	// Functions returning a value when things go wrong.
	template<typename T> inline T GetDefaultValue() {
		return 0;
	}
	template<> inline std::string GetDefaultValue<std::string>() {
		return "null";
	}

	// Functions handling the conversion from the lua state into the values we want to use.
	template<typename T> T GetValueFromLua(const std::string & variableName) {
		return 0;
	}
	template<> inline int GetValueFromLua(const std::string & variableName) {
		if (!lua_isnumber(L, -1)) {
			OutputError(variableName, "Not a number");
		}
		return (int)lua_tonumber(L, -1);
	}
	template<> inline float GetValueFromLua(const std::string& variableName) {
		if (!lua_isnumber(L, -1)) {
			OutputError(variableName, "Not a number");
		}
		return (float)lua_tonumber(L, -1);
	}
	template<> inline bool GetValueFromLua(const std::string& variableName) {
		return (bool)lua_toboolean(L, -1);
	}
	template<> inline std::string GetValueFromLua(const std::string& variableName) {
		if (!lua_isstring(L, -1)) {
			OutputError(variableName, "Not a string");
		}
		return (std::string)lua_tostring(L, -1);
	}

};

#endif