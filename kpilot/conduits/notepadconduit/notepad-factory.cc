/* KPilot
**
** Copyright (C) 2004 by Adriaan de Groot, Joern Ahrens
**
** This file defines the factory for the notepad-conduit plugin.
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

#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <qlineedit.h>

#include "pluginfactory.h"

#include "notepad-conduit.h"     // Conduit action
#include "notepad-setup.h"
#include "notepadconduit.h"     // Settings class

//----------------------------------------------------------------------------
// Conduit Configuration
//----------------------------------------------------------------------------
class NotepadConduitConfig : public ConduitConfigBase
{
public:
	NotepadConduitConfig(QWidget *parent=0L, const char *n=0L);
	virtual void commit();
	virtual void load();
	static ConduitConfigBase *create(QWidget *p, const char *n)
	{
		return new NotepadConduitConfig(p, n);
	};

protected:
	NotepadWidget *fConfigWidget;
} ;

static KAboutData *createAbout()
{
	FUNCTIONSETUP;

	KAboutData *fAbout = new KAboutData("NotepadConduit",
		I18N_NOOP("Saves notepads to png files"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Notepad Conduit for KPilot"),
		KAboutData::License_LGPL,
		"(C) 2004, Joern Ahrens");
	fAbout->addAuthor("Joern Ahrens",
		I18N_NOOP("Primary Author"),
		"kde@jokele.de",
		"http://www.jokele.de/");
	fAbout->addCredit("Adriaan de Groot");
	fAbout->addCredit("Angus Ainslies",
		I18N_NOOP("Notepad conduit is based on Angus' read-notepad, part of pilot-link" ));
	return fAbout;
}


NotepadConduitConfig::NotepadConduitConfig(QWidget *p, const char *n) :
	ConduitConfigBase(p, n),
	fConfigWidget(new NotepadWidget(p))
{
	FUNCTIONSETUP;

	fConduitName = i18n("Notepad");
	ConduitConfigBase::addAboutPage(fConfigWidget->tabWidget, createAbout());
	fWidget=fConfigWidget;
	QObject::connect(fConfigWidget->fOutputDirectory, SIGNAL(textChanged(const QString&)),
		this, SLOT(modified()));
	fConfigWidget->fOutputDirectory->setMode(KFile::Directory |
											KFile::LocalOnly);
}

/* virtual */ void NotepadConduitConfig::commit()
{
	FUNCTIONSETUP;

	NotepadConduitSettings::setOutputDirectory(fConfigWidget->fOutputDirectory->url());
	NotepadConduitSettings::self()->writeConfig();
}

/* virtual */ void NotepadConduitConfig::load()
{
	FUNCTIONSETUP;

	NotepadConduitSettings::self()->readConfig();
	fConfigWidget->fOutputDirectory->setURL(NotepadConduitSettings::outputDirectory());
	fModified=false;
}

extern "C"
{

void *init_conduit_notepad()
{
	return new ConduitFactory<NotepadConduitConfig,NotepadConduit>(0,"abbrowserconduit");
}

}

