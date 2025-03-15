#pragma once
#include "scripting/esl.h"
#include "scripting/esl_bind.h"

// the class that allows to change value inside callbacks
template <typename TType>
class LuaPropertyRef
{
public:
	LuaPropertyRef(TType& value);

	static void		Register(const esl::ScriptState& state);
	void			SetValue(const TType& newValue);
	const TType&	GetValue() const;

	TType& valueRef;
};

template <typename TType>
inline LuaPropertyRef<TType>::LuaPropertyRef(TType& value)
	: valueRef(value)
{
}

template <typename TType>
inline void LuaPropertyRef<TType>::SetValue(const TType& newValue)
{
	valueRef = newValue;
}

template <typename TType>
inline const TType& LuaPropertyRef<TType>::GetValue() const
{
	return valueRef;
}

template <typename TType>
inline void LuaPropertyRef<TType>::Register(const esl::ScriptState& state)
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

#define EQSCRIPT_BIND_PROPERTY_REF(Type) \
	EQSCRIPT_BIND_TYPE_NO_PARENT(LuaPropertyRef<Type>, "LuaPropertyRef<" #Type ">", BY_VALUE) \
	EQSCRIPT_TYPE_BEGIN(LuaPropertyRef<Type>) \
		MakeVariableExGetSet<_ESL_CLASS_MEMBER(GetValue), _ESL_CLASS_MEMBER(SetValue)>("value"), \
	EQSCRIPT_TYPE_END


// TODO: CONCAT define
#define MAKE_PROPERTY_REF(Type) state.RegisterClass<LuaPropertyRef<Type>>()