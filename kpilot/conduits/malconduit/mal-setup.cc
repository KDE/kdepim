/* MAL-setup.cc                      KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the setup dialog for the MAL-conduit plugin.
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
**
**
** Specific permission is granted for this code to be linked to libmal
** (this is necessary because the libmal license is not GPL-compatible).
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h> 
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kcombobox.h>
#include <knuminput.h>
#include <kpassdlg.h>

#include <kapplication.h>
#include <kconfig.h>

#include "mal-setup_dialog.h"

#include "mal-factory.h"
#include "mal-setup.moc"
#include "malconduitSettings.h"


MALWidgetSetup::MALWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new MALWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false,MALConduitFactory::about());

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
	fConduitName=i18n("MAL");
}

MALWidgetSetup::~MALWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void MALWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	MALConduitSettings::setSyncFrequency(
		fConfigWidget->syncTime->id(fConfigWidget->syncTime->selected()));
	
	// Proxy settings
	MALConduitSettings::setProxyType(
		fConfigWidget->proxyType->id(fConfigWidget->proxyType->selected()));
	MALConduitSettings::setProxyServer( fConfigWidget->proxyServerName->currentText() );

	if (fConfigWidget->proxyCustomPortCheck->isChecked() ) 
	{
		MALConduitSettings::setProxyPort( fConfigWidget->proxyCustomPort->value());
	}
	else 
	{
		MALConduitSettings::setProxyPort(0);
	}
	MALConduitSettings::setProxyUser( fConfigWidget->proxyUserName->text() );
	MALConduitSettings::setProxyPassword( fConfigWidget->proxyPassword->text() );

	// MAL Server settings (not yet possible!!!)
	MALConduitSettings::setMALServer( fConfigWidget->malServerName->currentText() );
	
	if (fConfigWidget->malCustomPortCheck->isChecked() ) 
	{
		MALConduitSettings::setMALPort( fConfigWidget->malCustomPort->value());
	}
	else 
	{
		MALConduitSettings::setMALPort(0);
	}
	MALConduitSettings::setMALUser( fConfigWidget->malUserName->text() );
	MALConduitSettings::setMALPassword( fConfigWidget->malPassword->text() );
	
	MALConduitSettings::self()->writeConfig();
}



/* virtual */ void MALWidgetSetup::readSettings()
{
	FUNCTIONSETUP;
	MALConduitSettings::self()->readConfig();

	fConfigWidget->syncTime->setButton( MALConduitSettings::syncFrequency() );
	
	// Proxy settings
	fConfigWidget->proxyType->setButton(MALConduitSettings::proxyType());
	fConfigWidget->proxyServerName->setEditText(MALConduitSettings::proxyServer());
	
	int proxyPortNr=MALConduitSettings::proxyPort();
	if (proxyPortNr>0 && proxyPortNr<65536) 
	{
		fConfigWidget->proxyCustomPortCheck->setChecked(true);
		fConfigWidget->proxyCustomPort->setEnabled(true);
		fConfigWidget->proxyCustomPort->setValue(proxyPortNr);
	}
	fConfigWidget->proxyUserName->setText(MALConduitSettings::proxyUser());
	fConfigWidget->proxyPassword->setText(MALConduitSettings::proxyPassword());

	// MAL Server settings (not yet possible!!!)
	fConfigWidget->malServerName->setEditText(MALConduitSettings::mALServer());
	
	int malPortNr=MALConduitSettings::mALPort();
	if (malPortNr>0 && malPortNr<65536) 
	{
		fConfigWidget->malCustomPortCheck->setChecked(true);
		fConfigWidget->malCustomPort->setEnabled(true);
		fConfigWidget->malCustomPort->setValue(proxyPortNr);
	}
	fConfigWidget->malUserName->setText(MALConduitSettings::mALUser());
	fConfigWidget->malPassword->setText(MALConduitSettings::mALPassword());
}

