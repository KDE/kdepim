#ifndef MK_POP3_PROTO_H
#define MK_POP3_PROTO_H

/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "kio_proto.h"
#include <kurl.h>

class Pop3_Protocol : public KIO_Protocol
{
public:
	Pop3_Protocol()  {}
	virtual ~Pop3_Protocol() {}

	virtual KIO_Protocol * clone() const { return new Pop3_Protocol; }

	virtual bool connectionBased() const { return true; }
	
	virtual QString protocol() const { return "pop3"; }
	virtual QString configName() const { return "pop3"; }
	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return true; }
	virtual bool canReadMail() const { return true; }

	virtual bool hasMailbox() const { return false; }
	virtual bool hasAuth() const { return true; }
	virtual unsigned short defaultPort() const { return 110; }

	virtual DeleteTypeEnum deleteFunction() const { return get; }

	virtual QStringList authList() const { return QStringList::split( '|', "Plain|APOP", false ); }
	
	virtual void readSubjectKURL( KURL & kurl, KIO::MetaData & ) { kurl.setPath( kurl.path().replace( "/download/", "/headers/" ) ); }
	virtual void deleteMailKURL ( KURL & kurl, KIO::MetaData & ) { kurl.setPath( kurl.path().replace( "/download/", "/remove/" ) ); }
	virtual bool commitDelete () { return true; }
	virtual void deleteCommitKURL(KURL & kurl, KIO::MetaData & ) { kurl.setPath( "commit" ); }
};

#endif
