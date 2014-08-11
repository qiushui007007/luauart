/*--- xuuart_lua.c: lua的专用支持库

scp ./luauart.so root@192.168.2.1:/usr/lib/lua/
scp ./sharp2.lua root@192.168.2.1:/xutest/

-----------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "xuuart.h"

#include "lua/lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define DBG //printf
//--------------------------------------------------------------------------------

#define STACK_DEBUG

#ifdef STACK_DEBUG
#define STACK_LIST(L)	lua_stack_list(L)
#else
#define STACK_LIST(L)
#endif

static void lua_stack_list(lua_State *L)
{
	int i;
	int top = lua_gettop(L);

	DBG("lua stack: %d = ", top);
	for(i = 1; i <= top; i++){
		int t = lua_type(L, i);
		switch(t){
			case LUA_TSTRING:
				DBG("%s, ", lua_tostring(L,i));
				break;
			case LUA_TBOOLEAN:
				DBG(lua_toboolean(L, i) ? "true, " : "false, " );
				break;
			case LUA_TNUMBER:
				DBG("%g, ", lua_tonumber(L, i));
				break;
			default:
				DBG("%s, ", lua_typename(L, t));
				break;
		}
	}
	DBG("\n");
}

//-------- 自定义结构 Begin ---------------------------------------------------------------
//C语言不允许大小为0的数组，这个1只是一个占位符, newarray 中重新给数组分配空间
typedef struct NumArray {
	size_t size;
	double values[1];
} NumArray;

#define TYPE_NAME_ARRAY "LuaBook.array"

//检查第1个参数是否是一个有效的数组
static NumArray *checkarray (lua_State *L, unsigned char argpos) {
	void *ud = luaL_checkudata(L, argpos, TYPE_NAME_ARRAY);
	luaL_argcheck(L, ud != NULL, argpos, "`array' expected");
	return (NumArray *)ud;
}

//检查第2个参数, 并返回元素的指针
static double *getelem (lua_State *L) {
	NumArray *a = checkarray(L, 1);
	int index = luaL_checkint(L, 2);
	luaL_argcheck(L, 1 <= index && index <= a->size, 2, "index out of range");
	/* return element address */
	return &a->values[index - 1];	//lua中从1起始
}

static int newarray (lua_State *L) {
	int n = luaL_checkint(L, 1);
	//由于原始的结构中已经包含了一个元素的空间, 因此实际分配时从n中减去1
	size_t nbytes = sizeof(NumArray) + (n - 1)*sizeof(double);
	NumArray *a = (NumArray *)lua_newuserdata(L, nbytes);

	//将表出栈并将其设置为给定位置的对象的metatable, 即新的userdata
	luaL_getmetatable(L, TYPE_NAME_ARRAY);
  lua_setmetatable(L, -2);
	a->size = n;

	return 1;  /* new userdatum is already on the stack */
}

//array.set(a, i, n), a[i] = n
static int setarray (lua_State *L) {
#if 0
	NumArray *a = (NumArray *)lua_touserdata(L, 1);
	int index = luaL_checkint(L, 2);
	double value = luaL_checknumber(L, 3);

	luaL_argcheck(L, a != NULL, 1, "`array' expected");
	luaL_argcheck(L, 1 <= index && index <= a->size, 2, "index out of range");
	a->values[index-1] = value;

#else
	double newvalue = luaL_checknumber(L, 3);
  *getelem(L) = newvalue;
#endif

	return 0;
}

static int getarray (lua_State *L) {
#if 0
	NumArray *a = (NumArray *)lua_touserdata(L, 1);
	int index = luaL_checkint(L, 2);
	luaL_argcheck(L, a != NULL, 1, "'array' expected");
	luaL_argcheck(L, 1 <= index && index <= a->size, 2, "index out of range");
	lua_pushnumber(L, a->values[index-1]);

#else
	lua_pushnumber(L, *getelem(L));
#endif

	return 1;
}

static int getsize (lua_State *L) {
#if 0
	NumArray *a = (NumArray *)lua_touserdata(L, 1);
	luaL_argcheck(L, a != NULL, 1, "`array' expected");

#else
	NumArray *a = checkarray(L, 1);
#endif

	lua_pushnumber(L, a->size);

	return 1;
}

//一个有100K元素的数组大概占用800KB的内存,同样的条件由Lua 表实现的数组需要1.5MB的内存
static const struct luaL_reg arrayfunc [] = {
	{"new", newarray},
	{"set", setarray},
	{"get", getarray},
	{"size", getsize},
	{NULL, NULL}
};

int luaopen_array (lua_State *L) {
	luaL_newmetatable(L, TYPE_NAME_ARRAY);	//新类型名,必须唯一
  //luaL_openlib(L, "array", arrayfunc, 0);
  luaL_register(L, "array", arrayfunc);

  //用传统的 a[] 来操作数组
  /* now the stack has the metatable at index 1 and 'array' at index 2 */
  //STACK_LIST(L);							//lua stack: 2 = table, table,
  lua_pushstring(L, "__index");
  lua_pushstring(L, "get");		//lua stack: 4 = table, table, __index, get,
  lua_gettable(L, 2); 				// get array.get, lua stack: 4 = table, table, __index, function,
  lua_settable(L, 1); 				// metatable.__index = array.get, lua stack: 2 = table, table,

  lua_pushstring(L, "__newindex");
  lua_pushstring(L, "set");	//lua stack: 4 = table, table, __newindex, set,
  lua_gettable(L, 2); /* get array.set */
  lua_settable(L, 1); // metatable.__newindex = array.set, lua stack: 2 = table, table,

  return 1;
}

//-------- 自定义结构 End ------------------------------------------------------------

//---------------------------------------------------------------------------
static int mylua_open (lua_State *L)
{
  int portnum;

  const char *com = lua_tostring(L, 1);
  if ((portnum = OpenCOMEx(com)) < 0) {
      //OWERROR(OWERROR_OPENCOM_FAILED);
      fprintf(stderr, "OPenCOMEx %s Error!\n", com);
      return -1;
  }
  lua_pushnumber(L, portnum);

  return 1;
}

static int mylua_close (lua_State *L)
{
  int portnum = lua_tonumber(L, 1);

  CloseCOM(portnum);

  return 1;
}

static int mylua_buad_set (lua_State *L)
{
  int portnum = lua_tonumber(L, 1);
  int arg_num = lua_tonumber(L, 2);
	//DBG("buad_set: %d\n", arg_num);

  SetBaudCOM(portnum, arg_num);

  return 1;
}

static int mylua_flush (lua_State *L)
{
  int portnum = lua_tonumber(L, 1);

  FlushCOM(portnum);

  return 1;
}

static int mylua_timeout (lua_State *L)
{
  int num = lua_tonumber(L, 1);

  timeout_put(num);

  return 1;
}

static int mylua_tx_rx(lua_State *L)
{
  char pc_ret[200];

  int portnum = lua_tonumber(L, 1);
  const char *pc_send = lua_tostring(L, 2);
  int len = lua_tonumber(L, 3);
  //DBG("tx: %s\n", pc_send);

  int len_ret = com_tx_rx(pc_ret, portnum, pc_send, len);

  lua_pushnumber(L, len_ret);
  lua_pushlstring(L, pc_ret, len_ret);
  //lua_pushstring(L, pc_ret);

  return 2;
}

static int l_sharp2_get(lua_State *L)
{
  const char *pc_com = lua_tostring(L, 1);

	float f_data = 0;
	char pc_ret[50];
  int len = sharp2_rx_full(&f_data, pc_ret, pc_com);
  if (len = 0) f_data = 0;
  //DBG("get: %f, %s\n", f_data, pc_ret);

  lua_pushnumber(L, f_data);
  lua_pushstring(L, pc_ret);

  return 2;
}

static int l_reboot(lua_State *L)
{
 	do_reboot();

  return 0;
}

static int l_delay_ms (lua_State *L)
{
  int num = lua_tonumber(L, 1);

  delay_ms(num);

  return 0;
}
//-------------------------------------------------------------------------------------
//动态库的函数列表，将函数注册为双引号内部的名字，调用时用双引号内的名字来调函数
static const struct luaL_reg func[] = {
	{"open", mylua_open},
	{"close", mylua_close},
	{"timeout", mylua_timeout},
	{"buad_set", mylua_buad_set},
	{"flush", mylua_flush},
	{"tx_rx", mylua_tx_rx},
	{"sharp2_get", l_sharp2_get},
	{"reboot", l_reboot},
	{"delay_ms", l_delay_ms},

	{NULL, NULL}
};

//其函数名必须为luaopen_xxx，此中xxx默示library名称。Lua代码require "xxx"须要与之对应
int luaopen_luauart(lua_State *L)
{
	//加载库和对应的函数，Lua通过双引号内的库名字和上面的函数名字，就可以调动态库内的函数了
	const char* lib_name = "luauart";

	//luaL_openlib(L, lib_name, func, 0);	//OK, 不建议用了
	luaL_register(L, lib_name, func);

	return 1;
}
