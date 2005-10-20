
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

#include "memvector-test.tcc"
#include "stringarray-test.tcc"
#include "match-test.tcc"
#include "stringset-test.tcc"
#include "leafdatavector-test.tcc"
#include "ifile-test.tcc"
#include "mempool-test.tcc"
#include "tokenizer-test.tcc"
#include "create-test.tcc"
#include "logfile.h"

test_suite* init_unit_test_suite(  int argc, char* argv[] )
{
	using namespace indexlib::tests;
	redirectlog( "unittest.log" );
	test_suite* test = BOOST_TEST_SUITE(  "Master test suite" );

	test->add( memvector_test::get_suite() );
	test->add( stringarray_test::get_suite() );
	test->add( match_test::get_suite() );
	test->add( stringset_test::get_suite() );
	test->add( leafdatavector_test::get_suite() );
	test->add( ifile_test::get_suite() );
	test->add( mempool_test::get_suite() );
	test->add( tokenizer_test::get_suite() );
	test->add( create_test::get_suite() );

	return test;
}
