/* ListMaker-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the ListMaker-conduit plugin.
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

#include "options.h"

#include <kinstance.h>
#include <kaboutdata.h>

#include "ListMaker-factory.h"


extern "C"
{

void *init_libListMakerConduit() {
	FUNCTIONSETUP;
	return new ListMakerConduitFactory;
}

};



ListMakerConduitFactory::ListMakerConduitFactory(QObject *p, const char *n) :
	OrganizerConduitFactory(p,n) {
	FUNCTIONSETUP;

	fAbout = new KAboutData(n,
		I18N_NOOP("ListMaker Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the ListMaker Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2002, Reinhold F. Kainhofer");
	fAbout->addAuthor("Dan Pilone", I18N_NOOP("Original Author of KPilot and the VCal conduit"));
	fAbout->addAuthor("Preston Brown", I18N_NOOP("Original Author of the VCal conduit"));
	fAbout->addAuthor("Herwin-Jan Steehouwer", I18N_NOOP("Original Author of the VCal conduit"));
	fAbout->addAuthor("Adriaan de Groot", I18N_NOOP("Maintainer or KPilot"), "groot@kde.org", "http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addAuthor("Reinhold Kainhofer", I18N_NOOP("Original author and maintainer of this conduit"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com");
}

#include "ListMaker-factory.moc"



// $Log$
// Revision 1.4  2002/04/07 11:56:19  reinhold
// Last version before moving to KDE CVS
//
// Revision 1.3  2002/03/15 20:43:17  reinhold
// Fixed the crash on loading (member function not defined)...
//
// Revision 1.2  2002/03/13 22:14:40  reinhold
// GUI should work now...
//
// Revision 1.1  2002/03/10 23:59:17  reinhold
// Made the conduit compile...
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the  project manager / List manager conduit.
//
//

