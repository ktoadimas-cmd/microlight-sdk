#include "lua_state.h"
#include <iostream>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace ml {

LuaState::LuaState() {
    L_ = luaL_newstate();
    if (!L_) {
        std::cerr << "LuaState: cannot create state\n";
        return;
    }

    luaL_openlibs(L_);

    std::cout << "Lua initialized: " << LUA_RELEASE << "\n";
}

LuaState::~LuaState() {
    if (L_) {
        lua_close(L_);
        L_ = nullptr;
    }
}

bool LuaState::DoFile(const std::string& path) {
    if (!L_) return false;

    int r = luaL_dofile(L_, path.c_str());
    if (r != LUA_OK) {
        const char* err = lua_tostring(L_, -1);
        std::cerr << "Lua error [" << path << "]: "
                  << (err ? err : "unknown") << "\n";
        lua_pop(L_, 1);
        return false;
    }
    return true;
}

bool LuaState::DoString(const std::string& code, const std::string& chunkName) {
    if (!L_) return false;

    int r = luaL_loadbuffer(L_, code.c_str(), code.size(), chunkName.c_str());
    if (r != LUA_OK) {
        const char* err = lua_tostring(L_, -1);
        std::cerr << "Lua load error: " << (err ? err : "unknown") << "\n";
        lua_pop(L_, 1);
        return false;
    }

    r = lua_pcall(L_, 0, LUA_MULTRET, 0);
    if (r != LUA_OK) {
        const char* err = lua_tostring(L_, -1);
        std::cerr << "Lua error: " << (err ? err : "unknown") << "\n";
        lua_pop(L_, 1);
        return false;
    }
    return true;
}

void LuaState::RegisterFunction(const std::string& name, int (*fn)(lua_State*)) {
    if (!L_) return;
    lua_pushcfunction(L_, fn);
    lua_setglobal(L_, name.c_str());
}

} // namespace ml
