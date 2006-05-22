/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MK_POP3_PROTO_H
#define MK_POP3_PROTO_H

#include "kio_proto.h"
#include <kurl.h>

class Pop3_Protocol : public KIO_Protocol
{
public:
	Pop3_Protocol()  {}
	virtual ~Pop3_Protocol() {}

	virtual KIO_Protocol * clone() const { return new Pop3_Protocol; }

	virtual bool connectionBased() const { return true; }
	
	virtual QString protocol( bool ssl ) const { return ssl ? "pop3s" : "pop3"; }
	virtual QString configName() const { return "pop3"; }
	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return true; }
	virtual bool canReadMail() const { return true; }

	virtual unsigned short defaultPort( bool ssl ) const { return ssl?995:110; }

	virtual DeleteTypeEnum deleteFunction() const { return get; }

	virtual QStringList authList() const { return QStringList::split( '|', "Plain|APOP", false ); }
	
	virtual void readSubjectKURL( KURL & kurl, KIO::MetaData & ) const { kurl.setPath( kurl.path().replace( "/download/", "/headers/" ) ); }
	virtual void deleteMailKURL ( KURL & kurl, KIO::MetaData & ) const { kurl.setPath( kurl.path().replace( "/download/", "/remove/" ) ); }
	virtual bool commitDelete () const { return true; }
	virtual void deleteCommitKURL(KURL & kurl, KIO::MetaData & ) const { kurl.setPath( "commit" ); }

	virtual void configFillGroupBoxes( QStringList* ) const;
        virtual void configFields( QPtrVector< QWidget >* vector, const QObject*, QPtrList< AccountInput >* ) const;
        virtual void readEntries( QMap< QString, QString >*, QMap< QString, QString >* ) const;
        virtual void writeEntries( QMap< QString, QString >* ) const;
};

#endif
