/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef KS_PLUCKER_INTERFACE_H
#define KS_PLUCKER_INTERFACE_H

#include <dcopobject.h>
#include <kurl.h>

namespace KSPlucker {
class PluckerInterface : virtual public DCOPObject 
{
    K_DCOP
k_dcop:
    virtual ASYNC addPluckerUrl( KURL ) = 0;
    virtual ASYNC addPluckerFeed(KURL)  = 0;
};

}

#endif
