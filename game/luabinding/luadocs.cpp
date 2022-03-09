#include "game/pch.h"

enum EDocPropType
{
	DocProp_MemberFunc,
	DocProp_Property,
	DocProp_Enum
};

struct LuaDocProp
{
	EDocPropType type;
	String declName;
	String description;
};

struct LuaDocItem
{
	String namespaceName;
	String declName;
	String description;
	Array<LuaDocProp> members;
};

Array<LuaDocItem*> g_luaDoc_typeList;

CLuaDocumentation::NamespaceGuard::NamespaceGuard(const char* name /*= nullptr*/)
{
	m_name = name ? String::fromCString(name) : "_G";
}

CLuaDocumentation::NamespaceGuard::~NamespaceGuard()
{

}

CLuaDocumentation::TypeGuard::TypeGuard(NamespaceGuard& ns, const char* name /*= nullptr*/, const char* docText /*= nullptr*/)
	: m_ns(ns)
{
	m_item = new LuaDocItem();
	m_item->namespaceName = m_ns.m_name;

	if (name)
		Init(name, docText);

	g_luaDoc_typeList.append(m_item);
}

CLuaDocumentation::TypeGuard::~TypeGuard()
{
}

const char* CLuaDocumentation::TypeGuard::Init(const char* name, const char* docText /*= nullptr*/)
{
	m_item->declName = String::fromCString(name);
	m_item->description = docText ? String::fromCString(docText) : "";
	return name;
}

const char* CLuaDocumentation::TypeGuard::Enum(const char* name, const char* docText /*= nullptr*/)
{
	m_item->members.append(LuaDocProp{
		DocProp_Enum,
		String::fromCString(name),
		docText ? String::fromCString(docText) : ""
		});

	return name;
}

const char* CLuaDocumentation::TypeGuard::Property(const char* name, const char* docText /*= nullptr*/)
{
	m_item->members.append(LuaDocProp{
		DocProp_Property,
		String::fromCString(name),
		docText ? String::fromCString(docText) : ""
		});

	return name;
}

const char* CLuaDocumentation::TypeGuard::MemberFunc(const char* name, const char* docText /*= nullptr*/)
{
	m_item->members.append(LuaDocProp{
		DocProp_MemberFunc,
		String::fromCString(name),
		docText ? String::fromCString(docText) : ""
		});

	return name;
}

//--------------------------------------

void CLuaDocumentation::Lua_Init(sol::state& lua)
{
	auto& docsTable = lua["docs"].get_or_create<sol::table>();

	for (usize i = 0; i < g_luaDoc_typeList.size(); i++)
	{
		LuaDocItem* doc = g_luaDoc_typeList[i];

		auto& namespaceTable = docsTable[(char*)doc->namespaceName].get_or_create<sol::table>();

		auto& declTable = namespaceTable[(char*)doc->declName].get_or_create<sol::table>();
		declTable["description"] = (char*)doc->description;

		auto& membersTable = declTable["members"].get_or_create<sol::table>();
		for (usize j = 0; j < doc->members.size(); j++)
		{
			LuaDocProp& prop = doc->members[j];
			auto& propTable = membersTable[(char*)prop.declName].get_or_create<sol::table>();
			
			propTable["type"] = prop.type;
			propTable["desc"] = (char*)prop.description;
		}

		delete doc;
	}
}