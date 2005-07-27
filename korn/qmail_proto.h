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

#ifndef MK_QMAIL_PROTO_H
#define MK_QMAIL_PROTO_H

#include "kio_proto.h"

class QMail_Protocol : public KIO_Protocol
{
public:
	QMail_Protocol() {}
	virtual ~QMail_Protocol() {}

	virtual KIO_Protocol * clone() const { return new QMail_Protocol; }

	virtual bool connectionBased() const { return false; }

	virtual QString protocol() const { return "file"; }
	virtual QString configName() const { return "qmail"; }

	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return true; }
	virtual bool canReadMail() const { return true; }
	virtual bool fullMessage() const { return true; }

	virtual void recheckKURL( KURL &kurl, KIO::MetaData& ) const
		{ if( kurl.path().right( 1 ) == "/" )
		  	kurl.setPath( kurl.path() +  "new" );
		  else
		  	kurl.setPath( kurl.path() + "/new" );
		}
	 
	
	virtual QString mailboxName() const { return i18n( "Path:" ); }

	virtual void configFillGroupBoxes( QStringList* ) const;
        virtual void configFields( QPtrVector< QWidget >* vector, const QObject*, QPtrList< AccountInput >* ) const;
        virtual void readEntries( QMap< QString, QString >*, QMap< QString, QString >* ) const;
        virtual void writeEntries( QMap< QString, QString >* ) const;
};

#endif
