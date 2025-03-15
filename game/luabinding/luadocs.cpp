#include "core/core_common.h"
#include "luadocs.h"

enum EDocPropType
{
	DocProp_Method,
	DocProp_Property,
	DocProp_Enum
};

struct LuaDocProp
{
	EDocPropType type;
	EqString declName;
	EqString description;
};

struct LuaDocItem
{
	EqString namespaceName;
	EqString declName;
	EqString description;
	Array<LuaDocProp> members;
};

Array<LuaDocItem*> g_luaDoc_typeList;

CLuaDocumentation::NamespaceGuard::NamespaceGuard(const char* name /*= nullptr*/)
{
	m_name = name ? name : "_G";
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
	m_item->declName = name;
	m_item->description = docText;
	return name;
}

const char* CLuaDocumentation::TypeGuard::Enum(const char* name, const char* docText /*= nullptr*/)
{
	m_item->members.append(LuaDocProp{
		DocProp_Enum,
		name,
		docText
	});

	return name;
}

const char* CLuaDocumentation::TypeGuard::Property(const char* name, const char* docText /*= nullptr*/)
{
	m_item->members.append(LuaDocProp{
		DocProp_Property,
		name,
		docText
	});

	return name;
}

const char* CLuaDocumentation::TypeGuard::MemberFunc(const char* name, const char* docText /*= nullptr*/)
{
	m_item->members.append(LuaDocProp{
		DocProp_Method,
		name,
		docText
	});

	return name;
}


//--------------------------------------
void CLuaDocumentation::Lua_Init(const esl::ScriptState& state)
{
	auto& docsTable = lua["docs"].get_or_create<sol::table>();

	for (int i = 0; i < g_luaDoc_typeList.numElem(); i++)
	{
		LuaDocItem* doc = g_luaDoc_typeList[i];

		auto& namespaceTable = docsTable[(char*)doc->namespaceName].get_or_create<sol::table>();

		auto& declTable = namespaceTable[(char*)doc->declName].get_or_create<sol::table>();
		declTable["description"] = (char*)doc->description;

		auto& membersTable = declTable["members"].get_or_create<sol::table>();
		for (int j = 0; j < doc->members.numElem(); j++)
		{
			LuaDocProp& prop = doc->members[j];
			auto& propTable = membersTable[prop.declName.ToCString()].get_or_create<sol::table>();
			
			propTable["type"] = prop.type;
			propTable["desc"] = prop.description;
		}

		delete doc;
	}
}