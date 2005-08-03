#ifndef LPC_TOKENIZER_H1118429480_INCLUDE_GUARD_
#define LPC_TOKENIZER_H1118429480_INCLUDE_GUARD_

#include <vector>
#include <string>
#include <memory>
#include <assert.h>

namespace indexlib { namespace detail {

class tokenizer {
	public:
		virtual ~tokenizer() { }
		std::vector<std::string> string_to_words( const char* str ) {
			assert( str );
			return do_string_to_words( str );
		}
	
	private:
		virtual std::vector<std::string> do_string_to_words( const char* ) = 0;
};

std::auto_ptr<tokenizer> get_tokenizer( std::string );
}}



#endif /* LPC_TOKENIZER_H1118429480_INCLUDE_GUARD_ */
