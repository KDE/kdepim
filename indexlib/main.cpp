
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

#include "stringarray.h"
#include "leafdata.h"
#include "manager.h"
#include "mmap_manager.h"
#include "mempool.h"
#include "compressed.h"
#include "create.h"
#include "tokenizer.h"
#include <sstream>
#include <map>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <memory>
#include <string.h>

typedef std::auto_ptr<indexlib::index> index_smart;

index_smart get_index( std::string name ) {
	return indexlib::open( name.c_str(), indexlib::open_flags::create_quotes );
}

std::string read_stream( std::istream& in ) {
	std::string res;
	char c;
	while ( in.get( c ) ) res.push_back( c );
	return res;
}
std::string read_string( std::string file ) {
	if ( file == "-" ) return read_stream( std::cin );
	std::ifstream in( file.c_str() );
	return read_stream( in );
}

void usage( int argc, char* argv[], const std::map<std::string, int (*)( int, char** )>& commands ) {
	std::cout 
		<< argv[ 0 ] 
		<< " cmd [index]\n"
		<< "Possible Commands:\n\n";

	for ( std::map<std::string, int (*)( int, char** )>::const_iterator first = commands.begin(), past = commands.end(); first != past; ++first ) {
		std::cout << '\t' << first->first << '\n';
	}
	std::cout << std::endl;
}

int debug( int argc, char* argv[] ) {
	using namespace indexlib;
	using namespace indexlib::detail;
	std::string type = argv[ 2 ];
	std::string argument = argv[ 3 ];
	if ( type == "print.sa" ) {
		//nolog();
		std::cout << "stringarray:\n";
		stringarray sa( argument );
		sa.print( std::cout );
	} else if ( type == "print.compressed" ) {
		compressed_file file( argument );
		nolog();
		std::cout << "compressed_file:\n";
		file.print( std::cout );
	} else if ( type == "break_up" ) {
		std::auto_ptr<tokenizer> tok = get_tokenizer( "latin-1:european" );
		if ( !tok.get() ) {
			std::cerr << "Could not get tokenizer\n";
			return 1;
		}
		nolog();
		std::ostringstream whole_str;
		whole_str << std::ifstream( argument.c_str() ).rdbuf();
		std::vector<std::string> words = tok->string_to_words( whole_str.str().c_str() );
		for ( std::vector<std::string>::const_iterator cur = words.begin(), past = words.end(); cur != past; ++cur ) {
			std::cout << *cur << '\n';
		}
	} else {
		std::cerr << "Unknown function\n";
		return 1;
	}
	return 0;
}

int remove_doc( int argc, char* argv[] ) {
	if ( argc < 4 ) {
		std::cerr << "Filename argument for remove_doc is required\n";
		return 1;
	}
	index_smart t = get_index( argv[ 2 ] );
	t->remove_doc( argv[ 3 ] );
	return 0;
}

int maintenance( int argc, char* argv[] ) { 
	index_smart t = get_index( argv[ 2 ] );
	t->maintenance();
	return 0;
}

int add( int argc, char* argv[] ) {
	if ( argc < 4 ) {
		std::cerr <<
			"Input file argument is required\n"
			"Name is optional (defaults to filename)\n";
		return 1;
	}
	index_smart t = get_index( argv[ 2 ] );
	std::string input;
	if ( argv[ 4 ] ) input = argv[ 4 ];
	else input = argv[ 3 ];
	t->add( read_string( input ), argv[ 3 ] );

	return 0;
}

int search( int argc, char* argv[] ) {
	if ( argc < 4 ) {
		std::cerr << "Search string is required\n";
		return 1;
	}
	index_smart t = get_index( argv[ 2 ]  );
	std::vector<unsigned> files = t->search( argv[ 3 ] )->list();
	for ( std::vector<unsigned>::const_iterator first = files.begin(), past = files.end();
			first != past; ++first ) {
		std::cout << t->lookup_docname( *first ) << std::endl;
	}
	return 0;
}

int list( int argc, char* argv[] ) {
	index_smart t = get_index( argv[ 2 ] );

	unsigned ndocs = t->ndocs();
	for ( unsigned i = 0; i != ndocs; ++i ) {
		std::cout << t->lookup_docname( i ) << std::endl;
	}
	return 0;
}

int remove( int argc, char* argv[] ) {
	indexlib::remove( argv[ 2 ] );
}

	
int main( int argc, char* argv[]) try {
	//nolog();

	std::map<std::string, int (*)( int, char* [] )> handlers;
	handlers[ "debug" ] = &debug;
	handlers[ "remove" ] = &remove;
	handlers[ "remove_doc" ] = &remove_doc;
	handlers[ "maintenance" ] = &maintenance;
	handlers[ "add" ] = &add;
	handlers[ "search" ] = &search;
	handlers[ "list" ] = &list;

	if ( argc < 3 ) {
		usage( argc, argv, handlers );
		return 0;
	}


	int ( *handle )( int, char*[] ) = handlers[ argv[ 1 ] ];

	if ( handle ) return handle( argc, argv );
	else {
		std::cerr << "Unkown command: " << argv[ 1 ] << std::endl;
		return 1;

	}
} catch ( const char* msg ) {
	std::cerr << "Error: " << msg << std::endl;
	return 1;
} catch ( std::exception& e ) {
	std::cerr << "Std Error: " << e.what() << std::endl;
	return 1;
} catch ( ... ) {
	std::cerr << "Some Unspecified error\n";
	return 1;
}

