#ifndef DKTYPES_H
#define DKTYPES_H

#include <nstd/Base.hpp>

// Define some sized types
typedef unsigned char	ubyte;
typedef unsigned short	ushort;


// disable stupid deprecate warning
#pragma warning(disable : 4996)

#define INLINE		__inline

#ifdef __GNUC__
#define __forceinline __attribute__((always_inline))
#endif // __GNUC__

#define FORCEINLINE __forceinline

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

// Define some useful macros
#define MCHAR2(a, b) (a | (b << 8))
#define MCHAR4(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))

// quick swap function
template< class T >
inline void QuickSwap(T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}

#endif // DKTYPES_H
