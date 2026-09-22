#pragma once
#include <string>

struct lua_State;

namespace ml {

class LuaState {
public:
    LuaState();
    ~LuaState();

    LuaState(const LuaState&) = delete;
    LuaState& operator=(const LuaState&) = delete;

    // Выполнить файл. Возвращает true если успех.
    bool DoFile(const std::string& path);

    // Выполнить строку кода
    bool DoString(const std::string& code, const std::string& chunkName = "<string>");

    // Зарегистрировать C-функцию как глобальную
    void RegisterFunction(const std::string& name, int (*fn)(lua_State*));

    lua_State* Raw() const { return L_; }

private:
    lua_State* L_ = nullptr;
};

} // namespace ml
