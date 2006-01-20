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

#ifndef MK_IMAPS_PROTO_H
#define MK_IMAPS_PROTO_H

/**
 * @file
 *
 * This file contains the Imap_Protocol class
 */

#include "kio_proto.h"

//Looks very simular to Imap_Protocol, so I inheritanced it.

/**
 * This is the protocol for imaps connections.
 * It looks very simular to Imap_Protocol, so I inheritanced it.
 */
class Imaps_Protocol : public Imap_Protocol
{
public:
	/**
	 * Empty constructor
	 */
	Imaps_Protocol()  {}
	/**
	 * Empty destructor
	 */
	virtual ~Imaps_Protocol() {}

	/**
	 * This function makes a new KIO_Protocol from the type Imaps_Protocol.
	 *
	 * @return a new instance of this class
	 */
	virtual KIO_Protocol * clone() const { return new Imaps_Protocol; }
	
	/**
	 * Returns the name of the kioslave which should be used for this protocol.
	 * In this class, it always returns "imaps".
	 *
	 * @return in this class, always "imaps"
	 */
	virtual QString protocol() const { return "imaps"; }
	/**
	 * Returns the name such as it should be written in the configuration file.
	 * This class always is of type "imaps", so it should always write "imaps" as type to the configuration file,
	 * so this function always returns "imaps".
	 *
	 * @return in this class, always "imaps"
	 */
	virtual QString configName() const { return "imaps"; }
	
	/**
	 * This function returns the default port.
	 * In this class, it always returns 993, because 993 is the default port for imaps connections.
	 *
	 * @return in this class, always 993.
	 */
	virtual unsigned short defaultPort() const { return 993; }
};

#endif //MK_IMAPS_PROTO_H
