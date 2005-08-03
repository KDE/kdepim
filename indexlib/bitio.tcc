#include "boost-compat/static_assert.hpp"
#include "boost-compat/remove_cv.hpp"
#ifdef HAVE_BOOST
#include <boost/type_traits/is_same.hpp>
#endif


namespace byte_io {

	template <typename T>
	inline T no_const( const volatile T v ) {
		return v;
	}

	template<typename T>
	inline
	void write( unsigned char* out, const volatile T d ) {
		write( out, no_const( d ) );
	}
	

	template <typename T>
	inline
	T read( const unsigned char* out ) {
		//BOOST_STATIC_ASSERT( !( ::boost::is_same<T,typename  ::boost::remove_cv<T>::type>::value ) );
		return read<typename ::boost::remove_cv<T>::type>( out );
	}

	template<>
	inline
	void write<uint8_t>( unsigned char* out, uint8_t d ) {
		*out = d;
	}

	template<>
	inline
	uint8_t read<uint8_t>( const unsigned char* in ) {
		return *in;
	}

	template<>
	struct byte_lenght_struct<uint8_t> {
		static const int value = 1;
	};

	template<>
	inline
	void write<uint16_t>( unsigned char* out, uint16_t d ) {
		*out++ = ( ( d >> 0 ) & 0xff );
		*out++ = ( ( d >> 8 ) & 0xff );
	}

	template<>
	inline
	uint16_t read<uint16_t>( const unsigned char* in ) {
		uint16_t res = 0;
		res |= ( ( *in++ & 0xff ) << 0 );
		res |= ( ( *in++ & 0xff ) << 8 );
		return res;
	}

	template<>
	struct byte_lenght_struct<uint16_t> {
		static const int value = 2;
	};

	template<>
	inline
	void write<uint32_t>( unsigned char* out, uint32_t d ) {
		*out++ = ( ( d >>  0 ) & 0xff );
		*out++ = ( ( d >>  8 ) & 0xff );
		*out++ = ( ( d >> 16 ) & 0xff );
		*out++ = ( ( d >> 24 ) & 0xff );
	}

	template<>
	inline
	uint32_t read<uint32_t>( const unsigned char* in ) {
		uint32_t res = 0;
		res |= ( ( *in++ & 0xff ) <<  0 );
		res |= ( ( *in++ & 0xff ) <<  8 );
		res |= ( ( *in++ & 0xff ) << 16 );
		res |= ( ( *in++ & 0xff ) << 24 );
		return res;
	}
	template<>
	struct byte_lenght_struct<uint32_t> {
		static const int value = 4;
	};
}

