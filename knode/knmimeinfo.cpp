/***************************************************************************
                          knmimeinfo.cpp  -  description
                             -------------------
   
    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "knmimeinfo.h"

KNMimeInfo::KNMimeInfo()
{
	c_tMType=MTtext;
	c_tSType=STplain;
	c_tEncoding=ECsevenBit;
	c_tDisposition=DPinline;
	c_tCategory=CCmain;
	i_sReadable=true;	
}



KNMimeInfo::~KNMimeInfo()
{
}
