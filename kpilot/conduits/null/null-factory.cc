/* KPilot
**
** Copyright (C) 2001 by Dan Pilone <pilone@slac.com>
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

#include "ui_setup_base.h"
#include "null-conduit.h"
#include "nullSettings.h"


class NullConduitConfig : public ConduitConfigBase
{
Q_OBJECT
public:
	NullConduitConfig(QWidget *parent, const QVariantList &);
	virtual void commit();
	virtual void load();
protected:
	Ui::NullWidget fConfigWidget;
	KAboutData *fAbout;
} ;

#include "null-factory.moc"

NullConduitConfig::NullConduitConfig(QWidget *p, const QVariantList &n) :
	ConduitConfigBase(p,n)
{
	FUNCTIONSETUP;

	fWidget = new QWidget();
	fConfigWidget.setupUi( fWidget );

	fConduitName = i18nc("This is the Null conduit, used for testing", "Null");

	fAbout = new KAboutData("nullConduit", 0,
		ki18n("Null Conduit for KPilot"),
		KPILOT_VERSION,
		ki18n("Configures the Null Conduit for KPilot"),
		KAboutData::License_GPL,
		ki18n("(C) 2001, 2007, Adriaan de Groot"));
	fAbout->addAuthor(ki18n("Adriaan de Groot"),
		ki18n("Primary Author"),
		"groot@kde.org",
		"http://www.kpilot.org");

	ConduitConfigBase::addAboutPage(fConfigWidget.tabWidget,fAbout);
	QObject::connect(fConfigWidget.fLogMessage,SIGNAL(textChanged(const QString&)),
		this,SLOT(modified()));
}

/* virtual */ void NullConduitConfig::commit()
{
	FUNCTIONSETUP;

	NullConduitSettings::setLogMessage( fConfigWidget.fLogMessage->text() );
	NullConduitSettings::self()->writeConfig();
	unmodified();
}

/* virtual */ void NullConduitConfig::load()
{
	FUNCTIONSETUP;
	NullConduitSettings::self()->readConfig();
	fConfigWidget.fLogMessage->setText( NullConduitSettings::logMessage() );
	unmodified();
}


DECLARE_KPILOT_PLUGIN(kpilot_conduit_null,NullConduitConfig,NullConduit)

