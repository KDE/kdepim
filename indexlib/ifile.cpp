
/* This file is part of indexlib.
 * Copyright (C) 2005 Luís Pedro Coelho <luis@luispedro.org>
 *
 * Indexlib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation and available as file
 * GPL_V2 which is distributed along with indexlib.
 * 
 * Indexlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA
 * 
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of this program with any edition of
 * the Qt library by Trolltech AS, Norway (or with modified versions
 * of Qt that use the same license as Qt), and distribute linked
 * combinations including the two.  You must obey the GNU General
 * Public License in all respects for all of the code used other than
 * Qt.  If you modify this file, you may extend this exception to
 * your version of the file, but you are not obligated to do so.  If
 * you do not wish to do so, delete this exception statement from
 * your version.
 */

#include "ifile.h"
#include "logfile.h"
#include "path.h"
#include "result.h"
#include <algorithm>
#include <iterator>
#include <set>
#include <functional>
#include <string.h>
#include "format.h"
#include "boost-compat/next_prior.hpp"


ifile::ifile( std::string name ):
	docnames_( path_concat( name, "docnames" ) ),
	words_( path_concat( name, "words" ) ),
	stopwords_( path_concat( name, "stopwords" ) ),
	files_( path_concat( name, "files" ) ),
	tokenizer_( indexlib::detail::get_tokenizer( "latin-1:european" ) )
{
	//logfile() << format( "ifile::ifile( \"%s\" )\n" ) % name;
}

void ifile::remove( std::string name ) {
	stringarray::remove( path_concat( name, "docnames" ) );
	stringset::remove( path_concat( name, "words" ) );
	stringset::remove( path_concat( name, "stopwords" ) );
	leafdatavector::remove( path_concat( name, "files" ) );
}

void ifile::add( const char* str, const char* doc ) {
	using namespace boost;
	//logfile() << format( "ifile::add( %s, %s )\n" ) % str % doc;
	const unsigned docidx = docnames_.add( doc );
	files_.resize( docidx + 1 );
	std::vector<std::string> words = break_clean( str );
	for ( std::vector<std::string>::const_iterator first = words.begin(), past = words.end(); first != past; ++first ) {
		files_.add( words_.add( first->c_str() ) , docidx );
	}
}

void ifile::remove_doc( const char* doc ) {
	//logfile() << format( "%s( %s )\n" ) % __PRETTY_FUNCTION__ % doc;
	unsigned idx;
	for ( idx = 0; idx != ndocs(); ++idx ) {
		if ( lookup_docname( idx ) == doc ) break;
	}
	if ( idx == ndocs() ) return;
	//logfile() << format( "Removing %s\n" ) % idx;
	docnames_.erase( idx );
	files_.remove_references_to( idx );
	// TODO: remove from words_ too if that's the case
}

std::auto_ptr<indexlib::result> ifile::everything() const {
	std::vector<unsigned> res( ndocs() );
	for ( unsigned i = 0; i != ndocs(); ++i ) res[ i ] = i;
	return std::auto_ptr<indexlib::result>( new indexlib::detail::simple_result( res ) );
}

namespace {
inline
bool word_too_small( std::string str ) { return str.size() < 3; }
}

std::auto_ptr<indexlib::result> ifile::search( const char* str ) const {
	using namespace indexlib::detail;
	using indexlib::result;
	assert( str );
	if ( !*str ) return everything();
	std::vector<std::string> words = break_clean( str );
	if ( words.empty() ) return std::auto_ptr<result>( new empty_result );
	words.erase( std::remove_if( words.begin(), words.end(), &word_too_small ), words.end() );
	if ( words.empty() ) return everything();
	std::set<unsigned> values = find_word( words[ 0 ] );
	for ( std::vector<std::string>::const_iterator first = boost::next( words.begin() ), past = words.end();
			first != past;
			++first ) {
		std::set<unsigned> now = find_word( *first );
		// merge the two
		std::set<unsigned> next;
		std::set_intersection( now.begin(), now.end(), values.begin(), values.end(), std::inserter( next, next.begin() ) );
		next.swap( values );
	}
	std::auto_ptr<result> r(new simple_result( std::vector<unsigned>( values.begin(), values.end() ) ) );
	return r;
}

void ifile::maintenance() {
	//logfile() << __PRETTY_FUNCTION__ << '\n';
	calc_stopwords();
}

void ifile::calc_stopwords() {
	//logfile() << __PRETTY_FUNCTION__ << '\n';
	const unsigned needed = ndocs() / 4;
	stopwords_.clear();
	for ( stringset::const_iterator word = words_.begin(), past = words_.end(); word != past; ++word ) {
		logfile() << format( "%s(): \"%s\" %s\n" ) 
			% __PRETTY_FUNCTION__
			% *word
			% files_.get( word.id() ).size();
		if ( files_.get( word.id() ).size() >= needed ) {
			stopwords_.add( *word );
			//files_.erase( word.id() );
		}
	}
}

bool ifile::is_stop_word( std::string str ) const {
	return stopwords_.count( str.c_str() );
}

bool ifile::invalid_word( std::string str ) {
	return str.find_first_of( "0123456789" ) != std::string::npos || str.size() > 32;
}



std::set<unsigned> ifile::find_word( std::string word ) const {
	//logfile() << format( "ifile::find_word( \"%s\" ): " ) % word;

	std::set<unsigned> res;
	for ( std::pair<stringset::const_iterator,stringset::const_iterator> limits =  words_.upper_lower( word.c_str() );
		       limits.first != limits.second; ++limits.first) {
		std::vector<unsigned> here = files_.get( limits.first.id() );
		//logfile() << format( "in ifile::search( \"%s\" ) seeing %s.\n" ) % word % limits.first.id();
		//std::copy( here.begin(), here.end(), std::ostream_iterator<unsigned>( logfile(), " - " ) );
		//logfile() << "\n";
		res.insert( here.begin(), here.end() );
	}
	//logfile() << format( "%s docs found.\n" ) % res.size();
	return res;
}

std::vector<std::string> ifile::break_clean( const char* complete ) const {
	std::vector<std::string> words = tokenizer_->string_to_words( complete );
	std::sort( words.begin(), words.end() );
	words.erase( std::unique( words.begin(), words.end() ), words.end() );
	words.erase( std::remove_if( words.begin(), words.end(), &ifile::invalid_word ), words.end() );
	words.erase( std::remove_if( words.begin(), words.end(), std::bind1st( std::mem_fun( &ifile::is_stop_word ), this ) ), words.end() );
	return words;
}



