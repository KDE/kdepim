/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines the internal conduit "File Installer"
** that accepts drags of URLs containing Palm DBs, prcs, and
** such. It also does the HotSync part of installing files
** on the Pilot.
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
static const char *fileinstallwidget_id =
	"$Id$";

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <unistd.h>

#include <qlistbox.h>
#include <qstring.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qmultilineedit.h>
#include <qpixmap.h>

#include <kfiledialog.h>
#include <kurldrag.h>
#include <kiconloader.h>
#include <kiconview.h>
#include <kglobal.h>
#include <kurl.h>

#include "kpilotConfig.h"
#include "fileInstaller.h"


#include "fileInstallWidget.moc"

FileInstallWidget::FileInstallWidget(QWidget * parent,
	const QString & path) :
	PilotComponent(parent, "component_files", path),
	fSaveFileList(false),
	fInstaller(0L)
{
	FUNCTIONSETUP;

	QGridLayout *grid = new QGridLayout(this, 5, 5, SPACING);

	QLabel *label = new QLabel(i18n("Files to install:"), this);

	grid->addWidget(label, 1, 1);

	QPushButton *abutton;

	abutton = addButton = new QPushButton(i18n("Add File..."), this);
	connect(abutton, SIGNAL(clicked()), this, SLOT(slotAddFile()));
	grid->addWidget(abutton, 3, 1);
	QWhatsThis::add(abutton,
		i18n("<qt>Choose a file to add to the list of files to install.</qt>"));

	abutton = clearButton= new QPushButton(i18n("Clear List"), this);
	connect(abutton, SIGNAL(clicked()), this, SLOT(slotClearButton()));
	grid->addWidget(abutton, 4, 1);
	QWhatsThis::add(abutton,
		i18n("<qt>Clear the list of files to install. No files will be installed.</qt>"));

	fIconView = new KIconView(this);
	connect(fIconView, SIGNAL(dropped(QDropEvent *, const QValueList<QIconDragItem> &)),
		this, SLOT(slotDropEvent(QDropEvent *, const QValueList<QIconDragItem> &)));
	grid->addMultiCellWidget(fIconView, 1, 4, 2, 3);
	QWhatsThis::add(fIconView,
		i18n
		("<qt>This lists files that will be installed on the Pilot during the next HotSync. Drag files here or use the Add button.</qt>"));
	fIconView->setAcceptDrops(true);
	fIconView->viewport()->installEventFilter(this);

	grid->setRowStretch(2, 100);
	grid->setColStretch(2, 50);
	grid->setColStretch(2, 50);
	grid->addColSpacing(4, SPACING);
	grid->addRowSpacing(5, SPACING);

	fInstaller = new FileInstaller;
	connect(fInstaller, SIGNAL(filesChanged()),
		this, SLOT(refreshFileInstallList()));

	(void) fileinstallwidget_id;
}

FileInstallWidget::~FileInstallWidget()
{
	KPILOT_DELETE(fInstaller);
}

static inline bool pdbOrPrc(const QString &s)
{
	return s.endsWith(CSL1(".pdb"),false) || s.endsWith(CSL1(".prc"),false) ;
}

void FileInstallWidget::dragEnterEvent(QDragEnterEvent *event)
{
	FUNCTIONSETUP;

	KURL::List urls;
	if(!KURLDrag::decode(event, urls)) {
		event->accept(false);
		return;
	}

	KURL::List::const_iterator it;
	QString filename;
    for ( it = urls.begin(); it != urls.end(); ++it ) {
		filename = (*it).fileName();
		if(!pdbOrPrc(filename)) {
			event->accept(false);
			return;
		}
	}
	event->accept(true);
}

bool FileInstallWidget::eventFilter(QObject *watched, QEvent *event)
{
	FUNCTIONSETUP;

	if((watched == fIconView->viewport()) && (event->type() == QEvent::DragEnter)) {
		dragEnterEvent(static_cast<QDragEnterEvent*>(event));
		return true;
	}
	// We have to skip the DragMove event, because it seems to override the accept state,
	// when it is set to false by dragEnterEvent() (event->accept(false);)
	if((watched == fIconView->viewport()) && (event->type() == QEvent::DragMove)) {
		return true;
	}

	return false;
}

void FileInstallWidget::dropEvent(QDropEvent * drop)
{
	FUNCTIONSETUP;
	if (!shown) return;

	KURL::List list;

	if (!KURLDrag::decode(drop, list) || list.isEmpty())
		return;

#ifdef DEBUG
	DEBUGKPILOT << ": Got " << list.first().prettyURL() << endl;
#endif

	QStringList files;
	for(KURL::List::ConstIterator it = list.begin(); it != list.end(); ++it)
	{
	   if ((*it).isLocalFile())
	      files << (*it).path();
	}

	fInstaller->addFiles(files, this );
}

void FileInstallWidget::slotDropEvent(QDropEvent * drop, const QValueList<QIconDragItem> & /*lst*/)
{
	FUNCTIONSETUP;
	dropEvent(drop);
}

void FileInstallWidget::slotClearButton()
{
	FUNCTIONSETUP;
	fInstaller->clearPending();
}

void FileInstallWidget::showComponent()
{
	FUNCTIONSETUP;
	refreshFileInstallList();
}

void FileInstallWidget::slotAddFile()
{
	FUNCTIONSETUP;
	if (!shown) return;

	QStringList fileNames = KFileDialog::getOpenFileNames(
		QString::null, i18n("*.pdb *.prc|PalmOS Databases (*.pdb *.prc)"));

	for (QStringList::Iterator fileName = fileNames.begin(); fileName != fileNames.end(); ++fileName)
	{
		fInstaller->addFile(*fileName, this );
	}
}

bool FileInstallWidget::preHotSync(QString &)
{
	FUNCTIONSETUP;

	fIconView->setEnabled(false);
	fInstaller->setEnabled(false);
	addButton->setEnabled(false);
	clearButton->setEnabled(false);

	return true;
}

void FileInstallWidget::postHotSync()
{
	FUNCTIONSETUP;

	fInstaller->setEnabled(true);
	fIconView->setEnabled(true);
	addButton->setEnabled(true);
	clearButton->setEnabled(true);
	if (shown) refreshFileInstallList();
}


void FileInstallWidget::refreshFileInstallList()
{
	FUNCTIONSETUP;

	QStringList fileNames = fInstaller->fileNames();
	QPixmap kpilotIcon = KGlobal::iconLoader()->loadIcon(CSL1("kpilot"), KIcon::Desktop);

	fIconView->clear();

	for (QStringList::Iterator fileName = fileNames.begin(); fileName != fileNames.end(); ++fileName)
	{
		if(pdbOrPrc(*fileName))
		{
			new KIconViewItem(fIconView, *fileName, kpilotIcon);
		}
		else
		{
			new KIconViewItem(fIconView, *fileName);
		}
	}
}

