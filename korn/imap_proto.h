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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MK_IMAP_PROTO_H
#define MK_IMAP_PROTO_H

#include "kio_proto.h"
#include <kurl.h>

/*
 * With deleting and IMAP4 is a small problem: messages don't looks as deleted, as they
 * apear with their full body. By deletion, kio_imap marks the message with FLAGS.SILENT as \DELETED.
 * If there is a commit-function, it should be installed in this file.
 */

class Imap_Protocol : public KIO_Protocol
{
public:
	Imap_Protocol()  {}
	virtual ~Imap_Protocol() {}

	virtual KIO_Protocol * clone() const { return new Imap_Protocol; }

	virtual bool connectionBased() const { return true; }
	
	virtual QString protocol() const { return "imap"; }
	virtual QString configName() const { return "imap"; }
	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return false; } //See comment above class: metadata expunge=auto doesn't work.
	virtual bool canReadMail() const { return true; }

	virtual int fields() const { return server | port | username | password | mailbox | auth; }
	virtual int urlFields() const { return no_fields; }
	virtual unsigned short defaultPort() const { return 143; }

	virtual QStringList authList() const { return QStringList::split( '|', "*|LOGIN|ANONYMOUS|CRAM-MD5", false); }
	//Could not test did, my server don't support other authentication methods.

	virtual void recheckKURL    ( KURL &kurl, KIO::MetaData & ) { kurl.setQuery( "unseen" ); }
	virtual void readSubjectKURL( KURL &kurl, KIO::MetaData & ) { kurl.setPath( kurl.path() + ";section=ENVELOPE" ); }
	virtual void deleteMailConnectKURL( KURL &, KIO::MetaData & metadata ) { metadata.insert( "expunge", "auto" ); }
	
};

#endif
