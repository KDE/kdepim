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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
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
#ifndef QTOOLTIP_H
#include <qtooltip.h>
#endif
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
	fSaveFileList(false)
{
	FUNCTIONSETUP;

	QGridLayout *grid = new QGridLayout(this, 5, 5, SPACING);

	QLabel *label = new QLabel(i18n("Files To Install:"), this);

	grid->addWidget(label, 1, 1);

	QPushButton *abutton = new QPushButton(i18n("Clear List"), this);

	connect(abutton, SIGNAL(clicked()), this, SLOT(slotClearButton()));
	grid->addWidget(abutton, 3, 1);
	QToolTip::add(abutton,
		i18n
		("<qt>Clear the list of files to install. No files wil be installed.</qt>"));

	abutton = new QPushButton(i18n("Add File"), this);
	connect(abutton, SIGNAL(clicked()), this, SLOT(slotAddFile()));
	grid->addWidget(abutton, 4, 1);
	QToolTip::add(abutton,
		i18n
		("<qt>Choose a file to add to the list of files to install.</qt>"));

	fListBox = new QListBox(this);
	grid->addMultiCellWidget(fListBox, 1, 4, 2, 3);
	QToolTip::add(fListBox,
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

void FileInstallWidget::postHotSync()
{
	FUNCTIONSETUP;
	refreshFileInstallList();
}


void FileInstallWidget::refreshFileInstallList()
{
	FUNCTIONSETUP;

	fListBox->clear();
	fListBox->insertStringList(fInstaller->fileNames());
}


// $Log$
// Revision 1.20  2001/09/30 16:59:22  adridg
// Cleaned up preHotSync
//
// Revision 1.19  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.18  2001/09/06 22:33:43  adridg
// Cruft cleanup
//
// Revision 1.17  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.16  2001/04/14 15:21:35  adridg
// XML GUI and ToolTips
//
// Revision 1.15  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.14  2001/03/04 13:11:58  adridg
// Actually use the fileInstaller object
//
// Revision 1.13  2001/02/26 22:12:24  adridg
// Use Qt layout classes
//
// Revision 1.12  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.11  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.10  2001/02/05 20:55:07  adridg
// Fixed copyright headers for source releases. No code changed
//
