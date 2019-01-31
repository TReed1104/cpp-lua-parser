#include "LuaScript.hpp"

int main() {
	LuaScript script = LuaScript("content/engine_config.lua");
	
	int tileSizeX = script.Get<int>("config.window.tile_size.x");


	return 0;
}