#ifndef MK_PROCESS_PROTOCOL
#define MK_PROCESS_PROTOCOL

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

	virtual bool hasServer() const { return false; }
	virtual bool hasPort() const { return false; }
	virtual bool hasUsername() const { return false; }
	virtual bool hasMailbox() const { return true; }
	virtual bool hasPassword() const { return false; }

	virtual QString mailboxName() const { return i18n("Program: "); }
};

#endif
