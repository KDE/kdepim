/***************************************************************************
                          serialexception.cpp  -  description
                             -------------------
    begin                : Don Aug 1 2002
    copyright            : (C) 2002 by Selzer Michael
    email                : selzer@student.uni-kl.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "serialexception.h"

CasioPV::SerialException::SerialException( string message, unsigned int errorcode )
												 : BaseException( message, errorcode )
{
}
CasioPV::SerialException::~SerialException()
{
}
