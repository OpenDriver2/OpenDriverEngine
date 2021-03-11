#ifndef DKTYPES_H
#define DKTYPES_H

#include <stddef.h>
#include <string.h>

// disable stupid deprecate warning
#pragma warning(disable : 4996)

#define INLINE		__inline

#ifdef __GNUC__
#define __forceinline __attribute__((always_inline))
#endif // __GNUC__

#define FORCEINLINE __forceinline

// classname of the main application window
#define DARKTECH_WINDOW_CLASSNAME "Equilibrium_9826C328_598D_4C2E_85D4_0FF8E0310366"

// Define some sized types
typedef unsigned char	uint8;
typedef   signed char	int8;

typedef unsigned short	uint16;
typedef   signed short  int16;

typedef unsigned int	uint32;
typedef   signed int	int32;

typedef unsigned char	ubyte;
typedef unsigned short	ushort;
typedef unsigned int	uint;

typedef ptrdiff_t intptr;

#ifdef _WIN32
	typedef   signed __int64  int64;
	typedef unsigned __int64 uint64;
#else
	typedef   signed long long  int64;
	typedef unsigned long long uint64;
#endif

#if !defined( __TYPEINFOGEN__ ) && !defined( _lint ) && defined(_WIN32)	// pcLint has problems with assert_offsetof()

template<bool> struct compile_time_assert_failed;
template<> struct compile_time_assert_failed<true> {};
template<int x> struct compile_time_assert_test {};
#define compile_time_assert_join2( a, b )	a##b
#define compile_time_assert_join( a, b )	compile_time_assert_join2(a,b)
#define compile_time_assert( x )			typedef compile_time_assert_test<sizeof(compile_time_assert_failed<(bool)(x)>)> compile_time_assert_join(compile_time_assert_typedef_, __LINE__)

#define assert_sizeof( type, size )						compile_time_assert( sizeof( type ) == size )
#define assert_sizeof_8_byte_multiple( type )			compile_time_assert( ( sizeof( type ) &  7 ) == 0 )
#define assert_sizeof_16_byte_multiple( type )			compile_time_assert( ( sizeof( type ) & 15 ) == 0 )
#define assert_offsetof( type, field, offset )			compile_time_assert( offsetof( type, field ) == offset )
#define assert_offsetof_8_byte_multiple( type, field )	compile_time_assert( ( offsetof( type, field ) & 7 ) == 0 )
#define assert_offsetof_16_byte_multiple( type, field )	compile_time_assert( ( offsetof( type, field ) & 15 ) == 0 )

#else

#define compile_time_assert( x )
#define assert_sizeof( type, size )
#define assert_sizeof_8_byte_multiple( type )
#define assert_sizeof_16_byte_multiple( type )
#define assert_offsetof( type, field, offset )
#define assert_offsetof_8_byte_multiple( type, field )
#define assert_offsetof_16_byte_multiple( type, field )

#endif

// test for C++ standard
assert_sizeof( char, 1);
assert_sizeof( short, 2);
assert_sizeof( int, 4);
assert_sizeof( long, 4);
assert_sizeof( long long, 8);
assert_sizeof( float, 4);
assert_sizeof( double, 8);

#define ERROR_BUFFER_LENGTH 2048

class CEqException
{
public:
	CEqException( const char* text = "" )
	{
		strncpy( s_szError, text, ERROR_BUFFER_LENGTH );
		s_szError[ERROR_BUFFER_LENGTH-1] = 0;
	}

	const char*	GetErrorString() const
	{
		return s_szError;
	}

protected:

	int	GetErrorBufferSize()
	{
		return ERROR_BUFFER_LENGTH;
	}

private:
	char s_szError[ERROR_BUFFER_LENGTH];
};

// quick swap function
template< class T >
inline void QuickSwap(T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}

#endif // DKTYPES_H
