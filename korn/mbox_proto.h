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

#ifndef MK_MBOX_PROTO_H
#define MK_MBOX_PROTO_H

#include "kio_proto.h"

/*
 * Protocol for mbox
 */
class MBox_Protocol : public KIO_Protocol
{
public:
	MBox_Protocol() {}
	virtual ~MBox_Protocol() {}

	virtual KIO_Protocol * clone() const { return new MBox_Protocol; }

	virtual QString protocol() const { return "mbox"; }
	virtual QString configName() const { return "mbox"; }

	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return false; }
	virtual bool canReadMail() const { return true; }
	virtual bool fullMessage() const { return true; }

	virtual int fields() const { return no_fields; }
	virtual int urlFields() const { return mailbox; }
		
	virtual QString mailboxName() const { return i18n( "File:" ); }
	
	virtual void recheckKURL( KURL &, KIO::MetaData & ) { }
	virtual void readSubjectKURL( KURL &, KIO::MetaData & ) { }
	virtual void deleteMailKURL( KURL &, KIO::MetaData & )  { }
	virtual void readMailKURL( KURL &, KIO::MetaData & )    { }
};

#endif
