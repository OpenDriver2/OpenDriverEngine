#include "core/core_common.h"
#include "luadocs.h"

#include "scripting/esl_luaref.h"
#include "scripting/esl_bind.h"

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
	Array<LuaDocProp> members{ PP_SL };
};

static Array<LuaDocItem*> g_luaDoc_typeList(PP_SL);

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
	m_item = PPNew LuaDocItem();
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
	esl::LuaTable docsTable = state.CreateTable();
	state.SetGlobal("docs", docsTable);

	for (LuaDocItem* doc : g_luaDoc_typeList)
	{
		esl::LuaTable namespaceTable = *docsTable.Get<esl::LuaTable>(doc->namespaceName);
		if(!namespaceTable)
		{
			namespaceTable = state.CreateTable();
			docsTable.Set(doc->namespaceName, namespaceTable);
		}

		esl::LuaTable declTable = *namespaceTable.Get<esl::LuaTable>(doc->declName);
		if (!declTable)
		{
			declTable = state.CreateTable();
			namespaceTable.Set(doc->declName, declTable);
			declTable.Set("description", doc->description);
		}

		esl::LuaTable membersTable = *declTable.Get<esl::LuaTable>("members");
		if (!membersTable)
		{
			membersTable = state.CreateTable();
			declTable.Set("members", membersTable);
		}

		for (LuaDocProp& prop : doc->members)
		{
			esl::LuaTable propTable = state.CreateTable();
			membersTable.Set(prop.declName, propTable);
			propTable.Set("type", prop.type);
			propTable.Set("desc", prop.description);
		}

		delete doc;
	}
}