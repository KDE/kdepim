/* KPilot
**
** Copyright (C) 2004 by Adriaan de Groot <groot@kde.org>
** Copyright (C) 2004 by Joern Ahrens <joern@kpilot.org>
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
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <qlineedit.h>

#include "pluginfactory.h"

#include "notepad-conduit.h"     // Conduit action
#include "ui_notepad-setup.h"
#include "notepadconduit.h"     // Settings class


//----------------------------------------------------------------------------
// Conduit Configuration
//----------------------------------------------------------------------------

class NotepadWidget : public QWidget, public Ui::NotepadWidget
{
public:
  NotepadWidget( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};


class NotepadConduitConfig : public ConduitConfigBase
{
Q_OBJECT
public:
	NotepadConduitConfig(QWidget *parent=0L, const QVariantList &  = QVariantList());
	virtual void commit();
	virtual void load();
	static ConduitConfigBase *create(QWidget *p, const char *n)
	{
		return new NotepadConduitConfig(p, QVariantList() << n);
	};

protected:
	NotepadWidget *fConfigWidget;
} ;

#include "notepad-factory.moc"

static KAboutData *createAbout()
{
	FUNCTIONSETUP;

	KAboutData *fAbout = new KAboutData("NotepadConduit", 0,
		ki18n("Saves notepads to png files"),
		KPILOT_VERSION,
		ki18n("Configures the Notepad Conduit for KPilot"),
		KAboutData::License_LGPL,
		ki18n("(C) 2004, Joern Ahrens"));
	fAbout->addAuthor(ki18n("Joern Ahrens"),
		ki18n("Primary Author"),
		"kde@jokele.de",
		"http://www.jokele.de/");
	fAbout->addCredit(ki18n("Adriaan de Groot"));
	fAbout->addCredit(ki18n("Angus Ainslies"),
		ki18n("Notepad conduit is based on Angus' read-notepad, part of pilot-link" ));
	return fAbout;
}


NotepadConduitConfig::NotepadConduitConfig(QWidget *p, const QVariantList &) :
	ConduitConfigBase(p),
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

	NotepadConduitSettings::setOutputDirectory(fConfigWidget->fOutputDirectory->url().url());
	NotepadConduitSettings::self()->writeConfig();
}

/* virtual */ void NotepadConduitConfig::load()
{
	FUNCTIONSETUP;

	NotepadConduitSettings::self()->readConfig();
	fConfigWidget->fOutputDirectory->setUrl(NotepadConduitSettings::outputDirectory());
	fModified=false;
}




DECLARE_KPILOT_PLUGIN(kpilot_conduit_notepad,NotepadConduitConfig,NotepadConduit)


