/* null-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include "setup_base.h"
#include "null-conduit.h"
#include "null-factory.moc"


extern "C"
{

void *init_libnullconduit()
{
	return new NullConduitFactory;
}

} ;

static QString nullname()
{
	FUNCTIONSETUP;
	return i18n("Null");
}

class NullConduitConfig : public ConduitConfigBase
{
public:
	NullConduitConfig(QWidget *parent=0L, const char *n=0L);
	virtual void commit(KConfig *);
	virtual void load(KConfig *);
	virtual QString conduitName() const;
protected:
	NullWidget *fConfigWidget;
} ;

QString NullConduitConfig::conduitName() const
{
	FUNCTIONSETUP;
	return nullname();
}

NullConduitConfig::NullConduitConfig(QWidget *p, const char *n) :
	ConduitConfigBase(p,n),
	fConfigWidget(new NullWidget(p))
{
	UIDialog::addAboutPage(fConfigWidget->tabWidget,NullConduitFactory::about());
	fWidget=fConfigWidget;
	QObject::connect(fConfigWidget->fLogMessage,SIGNAL(textChanged(const QString&)),
		this,SLOT(modified()));
	QObject::connect(fConfigWidget->fDatabases,SIGNAL(textChanged(const QString&)),
		this,SLOT(modified()));
	QObject::connect(fConfigWidget->fFailImmediately,SIGNAL(toggled(bool)),
		this,SLOT(modified()));
}

/* virtual */ void NullConduitConfig::commit(KConfig *fConfig)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Message="
		<< fConfigWidget->fLogMessage->text()
		<< endl;
	DEBUGCONDUIT << fname
		<< ": Databases="
		<< fConfigWidget->fDatabases->text()
		<< endl;
#endif

	KConfigGroupSaver s(fConfig,NullConduitFactory::group);

	fConfig->writeEntry(NullConduitFactory::message,fConfigWidget->fLogMessage->text());
	fConfig->writeEntry(NullConduitFactory::databases,fConfigWidget->fDatabases->text());
	fConfig->writeEntry(NullConduitFactory::failImmediately,
		fConfigWidget->fFailImmediately->isChecked());
}

/* virtual */ void NullConduitConfig::load(KConfig *fConfig)
{
	FUNCTIONSETUP;

	KConfigGroupSaver s(fConfig,NullConduitFactory::group);

	fConfigWidget->fLogMessage->setText(
		fConfig->readEntry(NullConduitFactory::message,i18n("KPilot was here!")));
	fConfigWidget->fDatabases->setText(
		fConfig->readEntry(NullConduitFactory::databases));
	fConfigWidget->fFailImmediately->setChecked(
		fConfig->readBoolEntry(NullConduitFactory::failImmediately,false));

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Read Message="
		<< fConfigWidget->fLogMessage->text()
		<< endl;
	DEBUGCONDUIT << fname
		<< ": Read Database="
		<< fConfigWidget->fDatabases->text()
		<< endl;
#endif

	fModified=false;
}

/* static */ const char * const NullConduitFactory::group = "Null-conduit";
const char * const NullConduitFactory::databases = "Databases" ;
const char * const NullConduitFactory::message = "LogMessage";
const char * const NullConduitFactory::failImmediately = "FailNow";

KAboutData *NullConduitFactory::fAbout = 0L;
NullConduitFactory::NullConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("nullconduit");
	fAbout = new KAboutData("nullConduit",
		I18N_NOOP("Null Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Null Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Primary Author"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
}

NullConduitFactory::~NullConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *NullConduitFactory::createObject( QObject *p,
	const char *n,
	const char *c,
	const QStringList &a)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Creating object of class "
		<< c
		<< endl;
#endif

	if (qstrcmp(c,"ConduitConfigBase")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);
		if (w)
		{
			return new NullConduitConfig(w);
		}
		else
		{
			return 0L;
		}
	}
	else
	if (qstrcmp(c,"ConduitConfig")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w)
		{
			return new NullWidgetSetup(w,n,a);
		}
		else 
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< ": Couldn't cast parent to widget."
				<< endl;
#endif
			return 0L;
		}
	}
	else if (qstrcmp(c,"SyncAction")==0)
	{
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d)
		{
			return new NullConduit(d,n,a);
		}
		else
		{
			kdError() << k_funcinfo
				<< ": Couldn't cast to KPilotDeviceLink"
				<< endl;
			return 0L;
		}
	}

	return 0L;
}

NullWidgetSetup::NullWidgetSetup(QWidget *w, const char *n, 
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new NullConduitConfig(this);
}

NullWidgetSetup::~NullWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void NullWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;
	fConfigWidget->commit(fConfig);
}

/* virtual */ void NullWidgetSetup::readSettings()
{
	FUNCTIONSETUP;
	fConfigWidget->load(fConfig);
}
