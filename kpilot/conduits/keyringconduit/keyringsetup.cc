/* KPilot
**
** Copyright (C) 2007 Bertjan Broeksema
** Copyright (C) 2002-2003 Reinhold Kainhofer
**
** This file defines the setup dialog for the Keyring-conduit plugin.
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "keyringsetup.h"

#include <kurlrequester.h>
#include <kaboutdata.h>

#include "options.h"


static KAboutData *createAbout()
{
	KAboutData *fAbout = new KAboutData( "keyringconduit", 0
		, ki18n( "Keyring Conduit for KPilot" )
		, KPILOT_VERSION
		, ki18n( "Configures the Keyring Conduit for KPilot" )
		, KAboutData::License_GPL
		, ki18n( "(C) 2007, Bertjan Broeksema" )
	);
	
	fAbout->addAuthor( ki18n( "Bertjan Broeksema" )
		, ki18n( "Primary Author" )
		, "b.broeksema@home.nl"
		, "http://bertjan.broeksemaatjes.nl"
	);
	
	return fAbout;
}

KeyringWidgetSetup::KeyringWidgetSetup( QWidget *w ) :
	ConduitConfigBase( w )
{
	FUNCTIONSETUP;

	fWidget = new QWidget();
	fUi.setupUi( fWidget );

	fConduitName = i18n( "Keyring Conduit" );
	fAbout = createAbout();
	ConduitConfigBase::addAboutPage( fUi.tabWidget, fAbout );
}

KeyringWidgetSetup::~KeyringWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void KeyringWidgetSetup::commit()
{
	FUNCTIONSETUP;
}

/* virtual */ void KeyringWidgetSetup::load()
{
	FUNCTIONSETUP;
}

/* static */ ConduitConfigBase *KeyringWidgetSetup::create( QWidget *w )
{
	return new KeyringWidgetSetup( w );
}

