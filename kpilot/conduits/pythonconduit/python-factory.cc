/* KPilot
**
** Copyright (C) 2004 by Adriaan de Groot
**
** This file defines the factory for the python-conduit plugin.
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

#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <qtextedit.h>

#include "uiDialog.h"

#include "python-conduit.h"     // Conduit action
#include "python-setup.h"
#include "python-factory.moc"
#include "pythonconduit.h"     // Settings class

extern "C"
{

void *init_conduit_python()
{
	return new PythonConduitFactory;
}

}

class PythonConduitConfig : public ConduitConfigBase
{
public:
	PythonConduitConfig(QWidget *parent=0L, const char *n=0L);
	virtual void commit();
	virtual void load();
protected:
	PythonWidget *fConfigWidget;
} ;

PythonConduitConfig::PythonConduitConfig(QWidget *p, const char *n) :
	ConduitConfigBase(p,n),
	fConfigWidget(new PythonWidget(p))
{
	FUNCTIONSETUP;
	fConduitName = i18n("Python");
	UIDialog::addAboutPage(fConfigWidget->tabWidget,PythonConduitFactory::about());
	fWidget=fConfigWidget;
	QObject::connect(fConfigWidget->fExpression,SIGNAL(textChanged()),
		this,SLOT(modified()));
}

/* virtual */ void PythonConduitConfig::commit()
{
	FUNCTIONSETUP;

	PythonConduitSettings::setExpression( fConfigWidget->fExpression->text() );
	PythonConduitSettings::self()->writeConfig();
}

/* virtual */ void PythonConduitConfig::load()
{
	FUNCTIONSETUP;
	PythonConduitSettings::self()->readConfig();

	fConfigWidget->fExpression->setText( PythonConduitSettings::expression() );

	fModified=false;
}

KAboutData *PythonConduitFactory::fAbout = 0L;
PythonConduitFactory::PythonConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("pythonconduit");
	fAbout = new KAboutData("pythonConduit",
		I18N_NOOP("Python Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Python Conduit for KPilot"),
		KAboutData::License_LGPL,
		"(C) 2004, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Primary Author"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/");
}

PythonConduitFactory::~PythonConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *PythonConduitFactory::createObject( QObject *p,
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
			return new PythonConduitConfig(w);
		}
		else
		{
			return 0L;
		}
	}

	if (qstrcmp(c,"SyncAction")==0)
	{
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d)
		{
			return new PythonConduit(d,n,a);
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


