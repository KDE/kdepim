#ifndef MK_IMAPS_PROTO_H
#define MK_IMAPS_PROTO_H

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

//Looks very simular to Imap_Protocol, so I inheritanced it.

class Imaps_Protocol : public Imap_Protocol
{
public:
	Imaps_Protocol()  {}
	virtual ~Imaps_Protocol() {}

	virtual KIO_Protocol * clone() const { return new Imaps_Protocol; }
	
	virtual QString protocol() const { return "newimaps"; }
	virtual QString configName() const { return "imaps"; }

	virtual unsigned short defaultPort() const { return 993; }
};

#endif
