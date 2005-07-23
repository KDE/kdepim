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

#ifndef MK_POP3S_PROTO_H
#define MK_POP3S_PROTO_H

//Looks very simulay to Pop3_Protocol, so I inherit it, and only overload the difference.

#include "pop3_proto.h"

class Pop3s_Protocol : public Pop3_Protocol
{
public:
	Pop3s_Protocol()  {}
	virtual ~Pop3s_Protocol() {}

	virtual KIO_Protocol * clone() const { return new Pop3s_Protocol; }
	
	virtual QString protocol() const { return "pop3s"; }
	virtual QString configName() const { return "pop3s"; }

	virtual unsigned short defaultPort() const { return 995; }
};

#endif
