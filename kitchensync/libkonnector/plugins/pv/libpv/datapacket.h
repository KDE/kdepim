/* **************************************************************************
                          datapacket.h  -  description
                             -------------------
    begin                : Don Sep 5 2002
    copyright            : (C) 2002 by Selzer Michael
    email                : selzer@student.uni-kl.de
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/**This struct holds a data packet as it is used for the communication with the PV
  *@author Selzer Michael
  */

#ifndef PVDATAPACKET_H
#define PVDATAPACKET_H

#include <string>


namespace CasioPV {

struct datapacket {
		/**
		   * The field code of the packet. Look in the file FieldCode.h for predefined values.
		   */
		unsigned int fieldCode;
		/**
		   * If a data packet is larger than 1024 bytes it has to be truncated in several packets.
		   * To show that it is continued this flag is set to true.
		   */
		bool continued;
		/**
		   * This holds the data.
		   */
		std::string data;
};

}; // namespace CasioPV

#endif
