/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2007 by Adriaan de Groot <groot@kde.org>
**
** This file defines the factory for the null-conduit plugin.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kconfig.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>

#include "pluginfactory.h"

#include "setup_base.h"
#include "null-conduit.h"
#include "nullSettings.h"


class NullConduitConfig : public ConduitConfigBase
{
public:
	NullConduitConfig(QWidget *parent=0L, const char *n=0L);
	virtual void commit();
	virtual void load();
protected:
	NullWidget *fConfigWidget;
	KAboutData *fAbout;
} ;

NullConduitConfig::NullConduitConfig(QWidget *p, const char *n) :
	ConduitConfigBase(p,n),
	fConfigWidget(new NullWidget(p))
{
	FUNCTIONSETUP;
	fConduitName = i18n("Null");
	fAbout = new KAboutData("nullConduit",
		I18N_NOOP("Null Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Null Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, 2007, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Primary Author"),
		"groot@kde.org",
		"http://www.kpilot.org");

	ConduitConfigBase::addAboutPage(fConfigWidget->tabWidget,fAbout);
	fWidget=fConfigWidget;
	QObject::connect(fConfigWidget->fLogMessage,SIGNAL(textChanged(const QString&)),
		this,SLOT(modified()));
}

/* virtual */ void NullConduitConfig::commit()
{
	FUNCTIONSETUP;

	DEBUGKPILOT << fname
		<< ": Message="
		<< fConfigWidget->fLogMessage->text()
		<< endl;

	NullConduitSettings::setLogMessage( fConfigWidget->fLogMessage->text() );
	NullConduitSettings::self()->writeConfig();
	unmodified();
}

/* virtual */ void NullConduitConfig::load()
{
	FUNCTIONSETUP;
	NullConduitSettings::self()->readConfig();

	fConfigWidget->fLogMessage->setText( NullConduitSettings::logMessage() );
#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Read Message="
		<< fConfigWidget->fLogMessage->text()
		<< endl;
#endif

	unmodified();
}



extern "C"
{

KPILOT_EXPORT unsigned long version_conduit_null = Pilot::PLUGIN_API;
KPILOT_EXPORT void *init_libconduit_null()
{
	return new ConduitFactory<NullConduitConfig,NullConduit>(0,"nullconduit");
}

}

