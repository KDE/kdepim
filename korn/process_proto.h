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

#ifndef MK_PROCESS_PROTOCOL
#define MK_PROCESS_PROTOCOL

#include "kio_proto.h"

class Process_Protocol : public KIO_Protocol
{
public:
	Process_Protocol() { }
	virtual ~Process_Protocol() { }

	virtual KIO_Protocol * clone() const { return new Process_Protocol; }

	virtual QString protocol() const { return "process"; }
	virtual QString configName() const { return "process"; }

	virtual bool canReadSubjects() const { return false; }
	virtual bool canDeleteMail() const { return false; }
	virtual bool canReadMail() const { return false; }
	virtual bool fullMessage() const { return false; }

	virtual int fields() const { return mailbox; }
	virtual int urlFields() const { return no_fields; }
	
	virtual QString mailboxName() const { return i18n("Program: "); }
};

#endif
