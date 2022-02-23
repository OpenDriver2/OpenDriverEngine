#include <sol/forward.hpp>

// the class that allows to change value inside callbacks
template <typename TType>
class LuaPropertyRef
{
public:
	LuaPropertyRef(TType& value);

	static void Lua_Init(sol::state& lua, const char* name);

	TType& valueRef;
};

template <typename TType>
inline LuaPropertyRef<TType>::LuaPropertyRef(TType& value)
	: valueRef(value)
{
}

template <typename TType>
inline void LuaPropertyRef<TType>::Lua_Init(sol::state& lua, const char* name)
{
	lua.new_usertype<LuaPropertyRef<TType>>(
		name,
		"value",

		sol::property(
			[](LuaPropertyRef<TType>& self) {
				return self.valueRef;
			},
			[](LuaPropertyRef<TType>& self, const TType& newValue) {
				self.valueRef = newValue;
			})
		);
}

// TODO: CONCAT define
#define MAKE_PROPERTY_REF(lua, type) LuaPropertyRef<type>::Lua_Init(lua, "LuaPropertyRef<" #type ">")