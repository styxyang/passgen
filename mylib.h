#ifndef MYLIB_H
#define MYLIB_H

#include <lua.h>

int luaopen_mylib(lua_State *L);

#define LUA_EXTRALIBS { "mylib", luaopen_mylib },

#endif /* MYLIB_H */
