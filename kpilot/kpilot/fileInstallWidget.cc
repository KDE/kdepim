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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <unistd.h>

#include <tqlistbox.h>
#include <tqstring.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <tqmultilineedit.h>
#include <tqpixmap.h>
#include <tqpopupmenu.h>

#include <kfiledialog.h>
#include <kurldrag.h>
#include <kiconloader.h>
#include <kiconview.h>
#include <kglobal.h>
#include <kurl.h>

#include "kpilotConfig.h"
#include "fileInstaller.h"


#include "fileInstallWidget.moc"

FileInstallWidget::FileInstallWidget(TQWidget * parent,
	const TQString & path) :
	PilotComponent(parent, "component_files", path),
	fSaveFileList(false),
	fInstaller(0L)
{
	FUNCTIONSETUP;

	TQGridLayout *grid = new TQGridLayout(this, 5, 5, SPACING);

	TQLabel *label = new TQLabel(i18n("Files to install:"), this);

	grid->addWidget(label, 1, 1);

	TQPushButton *abutton;

    abutton = addButton = new TQPushButton(i18n("Add File..."), this);
	connect(abutton, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotAddFile()));
	grid->addWidget(abutton, 3, 1);
	TQWhatsThis::add(abutton,
		i18n("<qt>Choose a file to add to the list of files to install.</qt>"));

	abutton = clearButton= new TQPushButton(i18n("Clear List"), this);
	connect(abutton, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotClearButton()));
	grid->addWidget(abutton, 4, 1);
	TQWhatsThis::add(abutton,
		i18n("<qt>Clear the list of files to install. No files will be installed.</qt>"));

	fIconView = new KIconView(this);
	connect(fIconView, TQT_SIGNAL(dropped(TQDropEvent *, const TQValueList<TQIconDragItem> &)),
		this, TQT_SLOT(slotDropEvent(TQDropEvent *, const TQValueList<TQIconDragItem> &)));
	grid->addMultiCellWidget(fIconView, 1, 4, 2, 3);
	TQWhatsThis::add(fIconView,
		i18n
		("<qt>This lists files that will be installed on the Pilot during the next HotSync. Drag files here or use the Add button.</qt>"));
	fIconView->setAcceptDrops(true);
    fIconView->setSelectionMode(TQIconView::Extended);
	fIconView->viewport()->installEventFilter(this);

	grid->setRowStretch(2, 100);
	grid->setColStretch(2, 50);
	grid->setColStretch(2, 50);
	grid->addColSpacing(4, SPACING);
	grid->addRowSpacing(5, SPACING);

	fInstaller = new FileInstaller;
	connect(fInstaller, TQT_SIGNAL(filesChanged()),
		this, TQT_SLOT(refreshFileInstallList()));

}

FileInstallWidget::~FileInstallWidget()
{
	KPILOT_DELETE(fInstaller);
}

static inline bool pdbOrPrc(const TQString &s)
{
	return s.endsWith(CSL1(".pdb"),false) || s.endsWith(CSL1(".prc"),false) ;
}

void FileInstallWidget::dragEnterEvent(TQDragEnterEvent *event)
{
	FUNCTIONSETUP;

	KURL::List urls;
	if(!KURLDrag::decode(event, urls)) {
		event->accept(false);
		return;
	}

	KURL::List::const_iterator it;
	TQString filename;
    for ( it = urls.begin(); it != urls.end(); ++it ) {
		filename = (*it).fileName();
		if(!pdbOrPrc(filename)) {
			event->accept(false);
			return;
		}
	}
	event->accept(true);
}

bool FileInstallWidget::eventFilter(TQObject *watched, TQEvent *event)
{
	FUNCTIONSETUP;

    if(watched == fIconView->viewport())
    {
        if(event->type() == TQEvent::DragEnter) {
    		dragEnterEvent(static_cast<TQDragEnterEvent*>(event));
            return true;
        }

        // We have to skip the DragMove event, because it seems to override the
        // accept state, when it is set to false by dragEnterEvent() (event->accept(false);)
        if(event->type() == TQEvent::DragMove) {
            return true;
        }

        if(event->type() == TQEvent::MouseButtonPress) {
            contextMenu(static_cast<TQMouseEvent*>(event));
        }
    }

	return false;
}

void FileInstallWidget::dropEvent(TQDropEvent * drop)
{
	FUNCTIONSETUP;
	if (!shown) return;

	KURL::List list;

	if (!KURLDrag::decode(drop, list) || list.isEmpty())
		return;

#ifdef DEBUG
	DEBUGKPILOT << ": Got " << list.first().prettyURL() << endl;
#endif

	TQStringList files;
	for(KURL::List::ConstIterator it = list.begin(); it != list.end(); ++it)
	{
	   if ((*it).isLocalFile())
	      files << (*it).path();
	}

	fInstaller->addFiles(files, this );
}

void FileInstallWidget::slotDropEvent(TQDropEvent * drop, const TQValueList<TQIconDragItem> & /*lst*/)
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

	TQStringList fileNames = KFileDialog::getOpenFileNames(
		TQString::null, i18n("*.pdb *.prc|PalmOS Databases (*.pdb *.prc)"));

	for (TQStringList::Iterator fileName = fileNames.begin(); fileName != fileNames.end(); ++fileName)
	{
		fInstaller->addFile(*fileName, this );
	}
}

bool FileInstallWidget::preHotSync(TQString &)
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

	TQStringList fileNames = fInstaller->fileNames();
	TQPixmap kpilotIcon = KGlobal::iconLoader()->loadIcon(CSL1("kpilot"), KIcon::Desktop);

	fIconView->clear();

	for (TQStringList::Iterator fileName = fileNames.begin(); fileName != fileNames.end(); ++fileName)
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

void FileInstallWidget::contextMenu(TQMouseEvent *event)
{
    FUNCTIONSETUP;

    if(event->button() == Qt::LeftButton)
        return;

    TQIconViewItem *item;
    TQStringList files;
    for(item = fIconView->firstItem(); item; item = item->nextItem())
    {
        if(item->isSelected())
            files.append(item->text());
    }

    TQPopupMenu popup(fIconView);

    item = fIconView->findItem(event->pos());
    if(item) {
        // Popup for the right clicked item
        popup.insertItem(i18n("Delete a single file item","Delete"), 10);
    }

    popup.insertItem(i18n("Delete selected files"), 11);
    if(files.empty())
        popup.setItemEnabled(11, false);

    int id = popup.exec(fIconView->viewport()->mapToGlobal(event->pos()));
    if(id == 10)
        fInstaller->deleteFile(item->text());
    else if(id == 11)
        fInstaller->deleteFiles(files);

}
