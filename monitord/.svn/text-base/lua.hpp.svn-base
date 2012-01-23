#ifndef LUA_HPP_
#define LUA_HPP_

#include "config.h"

#ifdef HAVE_LUA_H
	#ifdef HAVE_LUALIB_H
		// Alles für LUA scheint vorhanden zu sein
		#define LUA
	#endif
#endif

#ifdef LUA
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#endif 

#endif /*LUA_HPP_*/
