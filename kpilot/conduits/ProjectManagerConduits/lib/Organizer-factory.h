#ifndef _KPILOT_Organizer_FACTORY_H
#define _KPILOT_Organizer_FACTORY_H
/* Organizer-factory.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the Organizer-conduit plugin.
** It also defines the class for the behavior of the setup dialog.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "MultiDB-factory.h"


class OrganizerConduitFactory : public MultiDBConduitFactory {
Q_OBJECT
public:
	OrganizerConduitFactory(QObject * p= 0L,const char * n= 0L):MultiDBConduitFactory(p,n) {};
	virtual ~OrganizerConduitFactory(){};
	virtual void customConduitInfo();
} ;


#endif

