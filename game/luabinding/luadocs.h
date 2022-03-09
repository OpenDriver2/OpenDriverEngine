class CLuaDocumentation
{
public:
	class NamespaceGuard
	{
	public:
		NamespaceGuard(const char* name = nullptr);
		~NamespaceGuard();

		String m_name;
	};

	class TypeGuard
	{
		NamespaceGuard& m_ns;
		struct LuaDocItem* m_item;
	public:
		TypeGuard(NamespaceGuard& ns, const char* name = nullptr, const char* docText = nullptr);
		~TypeGuard();

		const char* Init(const char* name, const char* docText = nullptr);

		const char* Enum(const char* name, const char* docText = nullptr);
		const char* Property(const char* name, const char* docText = nullptr);
		const char* MemberFunc(const char* name, const char* docText = nullptr);
	};

	static void		Lua_Init(sol::state& lua);
};

//------------------------------------------------------------------------

#define LUADOC_GLOBAL()					CLuaDocumentation::NamespaceGuard	_n
#define LUADOC_NAMESPACE(name)			CLuaDocumentation::NamespaceGuard	_n(name)
#define LUADOC_TYPE(...)				CLuaDocumentation::TypeGuard		_t(_n, __VA_ARGS__)

#define LUADOC_ENUM(name, ...)			_t.Enum(name, __VA_ARGS__)
#define LUADOC_PROP(name, ...)			_t.Property(name, __VA_ARGS__)
#define LUADOC_MEMBER(name, ...)		_t.MemberFunc(name, __VA_ARGS__)

#define LUADOC_P(name, ...)				LUADOC_PROP(name, __VA_ARGS__)
#define LUADOC_M(name, ...)				LUADOC_MEMBER(name, __VA_ARGS__)
#define LUADOC_T(name, ...)				_t.Init(name, __VA_ARGS__)

#define LUA_BEGIN_ENUM(name)			using ENUM = name
#define LUA_ENUM(e, name, ...)			{ LUADOC_ENUM(name, __VA_ARGS__), ENUM::##e }