/* vcal-setup.cc                        KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the setup dialog for the vcal-conduit plugin.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qbuttongroup.h>
#include <klocale.h>

#include "korganizerConduit.h"
#include "vcal-factory.h"
#include "vcal-setup.h"

VCalWidgetSetup::VCalWidgetSetup(QWidget *w, const char *n) :
	VCalWidgetSetupBase(w,n)
{
	UIDialog::addAboutPage(fConfigWidget->tabWidget, VCalConduitFactoryBase::about());
	fConfigWidget->fSyncDestination->setTitle(i18n("Calendar Destination"));
	fConduitName=i18n("Calendar");

}

/* static */ ConduitConfigBase *VCalWidgetSetup::create(QWidget *w,const char *n)
{
	return new VCalWidgetSetup(w,n);
}
VCalConduitSettings*VCalWidgetSetup::config() { return VCalConduitFactory::config(); }
