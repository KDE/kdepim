/* fileInstallWidget.cc			KPilot
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



#ifndef QLISTBOX_H
#include <qlistbox.h>
#endif
#ifndef QSTRING_H
#include <qstring.h>
#endif
#ifndef QLABEL_H
#include <qlabel.h>
#endif
#ifndef QPUSHBT_H
#include <qpushbt.h>
#endif
#ifndef QDRAGOBJECT_H
#include <qdragobject.h>
#endif
#ifndef QLAYOUT_H
#include <qlayout.h>
#endif
#include <qwhatsthis.h>
#ifndef QMULTILINEEDIT_H
#include <qmultilineedit.h>
#endif

#ifndef _KFILEDIALOG_H
#include <kfiledialog.h>
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif
#ifndef _KPILOT_FILEINSTALLER_H
#include "fileInstaller.h"
#endif


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
	 
	 abutton = clearButton= new QPushButton(i18n("Clear List"), this);

	connect(abutton, SIGNAL(clicked()), this, SLOT(slotClearButton()));
	grid->addWidget(abutton, 3, 1);
	QWhatsThis::add(abutton,
		i18n
		("<qt>Clear the list of files to install. No files will be installed.</qt>"));

	abutton = addButton = new QPushButton(i18n("Add File"), this);
	connect(abutton, SIGNAL(clicked()), this, SLOT(slotAddFile()));
	grid->addWidget(abutton, 4, 1);
	QWhatsThis::add(abutton,
		i18n
		("<qt>Choose a file to add to the list of files to install.</qt>"));

	fListBox = new QListBox(this);
	grid->addMultiCellWidget(fListBox, 1, 4, 2, 3);
	QWhatsThis::add(fListBox,
		i18n
		("<qt>This lists files that will be installed on the Pilot during the next HotSync. Drag files here or use the Add button.</qt>"));

	grid->setRowStretch(2, 100);
	grid->setColStretch(2, 50);
	grid->setColStretch(2, 50);
	grid->addColSpacing(4, SPACING);
	grid->addRowSpacing(5, SPACING);

	fInstaller = new FileInstaller;
	connect(fInstaller, SIGNAL(filesChanged()),
		this, SLOT(refreshFileInstallList()));

	setAcceptDrops(true);

	(void) fileinstallwidget_id;
}

FileInstallWidget::~FileInstallWidget()
{
	KPILOT_DELETE(fInstaller);
}

void FileInstallWidget::dragEnterEvent(QDragEnterEvent * event)
{
	FUNCTIONSETUP;
	event->accept(QUriDrag::canDecode(event));
}


void FileInstallWidget::dropEvent(QDropEvent * drop)
{
	FUNCTIONSETUP;

	QStrList list;

	QUriDrag::decode(drop, list);

#ifdef DEBUG
	DEBUGKPILOT << ": Got " << list.first() << endl;
#endif

	if (list.first() != 0L)
	{
		fInstaller->addFiles(list);
	}
}

void FileInstallWidget::slotClearButton()
{
	FUNCTIONSETUP;
	fInstaller->clearPending();
}

void FileInstallWidget::initialize()
{
	FUNCTIONSETUP;
	refreshFileInstallList();
}

void FileInstallWidget::slotAddFile()
{
	FUNCTIONSETUP;

	QString fileName = KFileDialog::getOpenFileName();

	if (!fileName.isEmpty())
	{
		fInstaller->addFile(fileName);
	}
}

bool FileInstallWidget::preHotSync(QString &)
{
	FUNCTIONSETUP;
	
	fListBox->setEnabled(false);
	fInstaller->setEnabled(false);
	addButton->setEnabled(false);
	clearButton->setEnabled(false);
	
	return true;
}

void FileInstallWidget::postHotSync()
{
	FUNCTIONSETUP;
	fInstaller->setEnabled(true);
	fListBox->setEnabled(true);
	addButton->setEnabled(true);
	clearButton->setEnabled(true);
	refreshFileInstallList();
}


void FileInstallWidget::refreshFileInstallList()
{
	FUNCTIONSETUP;

	fListBox->clear();
	fListBox->insertStringList(fInstaller->fileNames());
}


