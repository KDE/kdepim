/* perl-factory.cc                      KPilot
**
** Copyright (C) 2004 by Adriaan de Groot
**
** This file defines the factory for the perl-conduit plugin.
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
#include <qlineedit.h>

#include "notepad-conduit.h"     // Conduit action
#include "notepad-setup.h"
#include "notepad-factory.moc"
#include "notepadconduit.h"     // Settings class

extern "C"
{

void *init_conduit_notepad()
{
	return new PerlConduitFactory;
}

}

class PerlConduitConfig : public ConduitConfigBase
{
public:
	PerlConduitConfig(QWidget *parent=0L, const char *n=0L);
	virtual void commit();
	virtual void load();
	static ConduitConfigBase *create(QWidget *p,const char *n)
		{ return new PerlConduitConfig(p,n); } ;
protected:
	PerlWidget *fConfigWidget;
} ;

PerlConduitConfig::PerlConduitConfig(QWidget *p, const char *n) :
	ConduitConfigBase(p,n),
	fConfigWidget(new PerlWidget(p))
{
	FUNCTIONSETUP;
	fConduitName = i18n("Perl");
	UIDialog::addAboutPage(fConfigWidget->tabWidget,PerlConduitFactory::about());
	fWidget=fConfigWidget;
	QObject::connect(fConfigWidget->fExpression,SIGNAL(textChanged(const QString&)),
		this,SLOT(modified()));
}

/* virtual */ void PerlConduitConfig::commit()
{
	FUNCTIONSETUP;

	NotepadConduitSettings::setExpression( fConfigWidget->fExpression->text() );
	NotepadConduitSettings::self()->writeConfig();
}

/* virtual */ void PerlConduitConfig::load()
{
	FUNCTIONSETUP;
	NotepadConduitSettings::self()->readConfig();

	fConfigWidget->fExpression->setText( NotepadConduitSettings::expression() );

	fModified=false;
}

KAboutData *PerlConduitFactory::fAbout = 0L;
PerlConduitFactory::PerlConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("notepadconduit");
	fAbout = new KAboutData("NotepadConduit",
		I18N_NOOP("Saves notepads to png files"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Notepad Conduit for KPilot"),
		KAboutData::License_LGPL,
		"(C) 2004, Joern Ahrens");
	fAbout->addAuthor("Joern Ahrens",
		I18N_NOOP("Primary Author"),
		"joern@kpilot.org",
		"http://www.jokele.de/");
}

PerlConduitFactory::~PerlConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *PerlConduitFactory::createObject( QObject *p,
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
			return new PerlConduitConfig(w);
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
			return new PerlConduit(d,n,a);
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


