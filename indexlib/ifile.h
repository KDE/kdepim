#ifndef _IFILE_INCLUDE_GUARD_LPC_56465465798732
#define _IFILE_INCLUDE_GUARD_LPC_56465465798732

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

#include "index.h"
#include "stringarray.h"
#include "stringset.h"
#include "leafdatavector.h"
#include "tokenizer.h"
#include <vector>
#include <set>
#include <string>

struct  ifile : public indexlib::index {
	public:
		ifile( std::string );
		virtual void add( const char* str, const char* doc );
		virtual std::auto_ptr<indexlib::result> search( const char* ) const;

		virtual unsigned ndocs() const { return docnames_.size(); }
		virtual std::string lookup_docname( unsigned idx ) const { return docnames_.get( idx ); }

		virtual void remove_doc( const char* doc );

		void maintenance();
		static void remove( std::string );
	private:
		std::set<unsigned> find_word( std::string ) const;
		std::vector<std::string> break_clean( const char* ) const;
		virtual std::auto_ptr<indexlib::result> everything() const;

		static bool invalid_word( std::string );
		bool is_stop_word( std::string ) const;

		void calc_stopwords();

		stringarray docnames_;
		stringset words_;
		stringset stopwords_;
		leafdatavector files_;
		std::auto_ptr<indexlib::detail::tokenizer> tokenizer_;
};

#endif /* _IFILE_INCLUDE_GUARD_LPC_56465465798732 */

