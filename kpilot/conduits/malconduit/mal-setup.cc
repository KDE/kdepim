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

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,MALConduitFactory::group());
	
	fConfig->writeEntry(MALConduitFactory::syncTime(),
		fConfigWidget->syncTime->id(fConfigWidget->syncTime->selected()));
	
	// Proxy settings
	fConfig->writeEntry(MALConduitFactory::proxyType(),
		fConfigWidget->proxyType->id(fConfigWidget->proxyType->selected()));
	fConfig->writeEntry(MALConduitFactory::proxyServer(), fConfigWidget->proxyServerName->currentText() );

	if (fConfigWidget->proxyCustomPortCheck->isChecked() ) 
	{
		fConfig->writeEntry(MALConduitFactory::proxyPort(), 	fConfigWidget->proxyCustomPort->value());
	}
	else 
	{
		fConfig->writeEntry(MALConduitFactory::proxyPort(), 0);
	}
	fConfig->writeEntry(MALConduitFactory::proxyUser(),  fConfigWidget->proxyUserName->text() );
	fConfig->writeEntry(MALConduitFactory::proxyPassword(), fConfigWidget->proxyPassword->text() );

	// MAL Server settings (not yet possible!!!)
	fConfig->writeEntry(MALConduitFactory::malServer(), fConfigWidget->malServerName->currentText() );
	
	if (fConfigWidget->malCustomPortCheck->isChecked() ) 
	{
		fConfig->writeEntry(MALConduitFactory::malPort(), fConfigWidget->malCustomPort->value());
	}
	else 
	{
		fConfig->writeEntry(MALConduitFactory::malPort(), 0);
	}
	fConfig->writeEntry(MALConduitFactory::malUser(),  fConfigWidget->malUserName->text() );
	fConfig->writeEntry(MALConduitFactory::malPassword(), fConfigWidget->malPassword->text() );
}



/* virtual */ void MALWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,MALConduitFactory::group());
	
	fConfigWidget->syncTime->setButton(fConfig->readNumEntry(MALConduitFactory::syncTime(), 0));
	
	// Proxy settings
	fConfigWidget->proxyType->setButton(fConfig->readNumEntry(MALConduitFactory::proxyType(), 0));
	fConfigWidget->proxyServerName->setEditText(fConfig->readEntry(MALConduitFactory::proxyServer(), QString()));
	
	int proxyPortNr=fConfig->readNumEntry(MALConduitFactory::proxyPort(), 0);
	if (proxyPortNr>0 && proxyPortNr<65536) 
	{
		fConfigWidget->proxyCustomPortCheck->setChecked(true);
		fConfigWidget->proxyCustomPort->setEnabled(true);
		fConfigWidget->proxyCustomPort->setValue(proxyPortNr);
	}
	fConfigWidget->proxyUserName->setText(fConfig->readEntry(MALConduitFactory::proxyUser(), QString()));
	fConfigWidget->proxyPassword->setText(fConfig->readEntry(MALConduitFactory::proxyPassword(), QString()));

	// MAL Server settings (not yet possible!!!)
	fConfigWidget->malServerName->setEditText(fConfig->readEntry(MALConduitFactory::malServer(), "sync.avantgo.com"));
	
	int malPortNr=fConfig->readNumEntry(MALConduitFactory::malPort(), 0);
	if (malPortNr>0 && malPortNr<65536) 
	{
		fConfigWidget->malCustomPortCheck->setChecked(true);
		fConfigWidget->malCustomPort->setEnabled(true);
		fConfigWidget->malCustomPort->setValue(proxyPortNr);
	}
	fConfigWidget->malUserName->setText(fConfig->readEntry(MALConduitFactory::malUser(), QString()));
	fConfigWidget->malPassword->setText(fConfig->readEntry(MALConduitFactory::malPassword(), QString()));
}

