#ifndef LPC_FORMAT_H1119018900_INCLUDE_GUARD_
#define LPC_FORMAT_H1119018900_INCLUDE_GUARD_

#if defined( HAVE_BOOST ) && defined( DEBUG )

#include <boost/format.hpp>

typedef boost::format format;
#else

struct null_format {
	explicit null_format( const char* ) { }

	template <typename T>
	null_format& operator % ( const T& ) { return *this; }
};

template <typename OutStream>
inline OutStream& operator << ( OutStream& out, const null_format& ) { return out; }

typedef null_format format;

#endif

#endif /* LPC_FORMAT_H1119018900_INCLUDE_GUARD_ */

