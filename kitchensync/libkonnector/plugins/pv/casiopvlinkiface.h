/***************************************************************************
                        casiopvlinkiface.h  -  description
                             -------------------
    begin                : Sun Oct 06 2002
    copyright            : (C) 2002 by Maurus Erni
    email                : erni@pocketviewer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CASIOPVLINKIFACE_H
#define CASIOPVLINKIFACE_H

#include <dcopobject.h>

class CasioPVLinkIface : virtual public DCOPObject
{
  K_DCOP
  k_dcop:

    virtual void getChangesDone(const QByteArray& stream) = 0;

    virtual void getAllEntriesDone(const QByteArray& stream) = 0;

    virtual void setChangesDone(const bool ok) = 0;

    virtual void setAllEntriesDone(const bool ok) = 0;

    virtual void errorPV(const QString& msg, const unsigned int errorcode) = 0;

};

#endif
