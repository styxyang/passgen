#ifndef MYLIB_H
#define MYLIB_H

#include <lua.h>

int luaopen_passgen(lua_State *L);

#define LUA_EXTRALIBS { "passgen", luaopen_passgen },

#endif /* MYLIB_H */
