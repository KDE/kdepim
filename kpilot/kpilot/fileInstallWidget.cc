/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
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

#include "options.h"

#include <QtGui/QDragEnterEvent>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QListWidgetItem>
#include <QtGui/QMenu>
#include <QtGui/QMouseEvent>
#include <QtGui/QPushButton>

#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "fileInstaller.h"

#include "fileInstallWidget.moc"
#include "ui_file_install_widget.h"

class FileInstallWidget::Private
{
public:
	Ui::FileInstallWidget fUi;
};

FileInstallWidget::FileInstallWidget( QWidget * parent )
	: ComponentPageBase( parent ), fP( new Private ), fSaveFileList(false)
		, fInstaller(0L)
{
	FUNCTIONSETUP;
	
	fP->fUi.setupUi( this );
	fP->fUi.fIconView->installEventFilter( this );
	
	fInstaller = new FileInstaller;

	connect( fP->fUi.fAddButton, SIGNAL( clicked() ), this, SLOT( slotAddFile() ) );
	connect( fP->fUi.fClearButton, SIGNAL( clicked() ), this
		, SLOT( slotClearButton() ) );
	connect( fP->fUi.fIconView, SIGNAL( dropEvent( QDropEvent* ) ),
		this, SLOT( slotDropEvent( QDropEvent* ) ) );
	connect( fInstaller, SIGNAL( filesChanged() ),
		this, SLOT( refreshFileInstallList() ) );

}

FileInstallWidget::~FileInstallWidget()
{
	KPILOT_DELETE(fInstaller);
}

static inline bool pdbOrPrc( const QString &s )
{
	return s.endsWith( CSL1( ".pdb" ), Qt::CaseInsensitive )
		|| s.endsWith( CSL1( ".prc" ), Qt::CaseInsensitive );
}

bool FileInstallWidget::eventFilter( QObject *watched, QEvent *event )
{
	FUNCTIONSETUP;

	if( watched == fP->fUi.fIconView )
	{
		if( event->type() == QEvent::DragEnter )
		{
			dragEnterEvent( static_cast<QDragEnterEvent*>( event ) );
			return true;
		}
		
		if( event->type() == QEvent::Drop )
		{
			 dropEvent( static_cast<QDropEvent*>(event) );
		}

		// We have to skip the DragMove event, because it seems to override the
		// accept state, when it is set to false by dragEnterEvent() (event->accept(false);)
		if( event->type() == QEvent::DragMove )
		{
			return true;
		}
		
		if( event->type() == QEvent::ContextMenu )
		{
			contextMenu( static_cast<QContextMenuEvent*>(event) );
		}
	}

	return false;
}

void FileInstallWidget::dragEnterEvent( QDragEnterEvent *event )
{
	FUNCTIONSETUP;

	if( !KUrl::List::canDecode( event->mimeData() ) )
	{
		DEBUGKPILOT << "Could not decode mime data";
		event->setAccepted(false);
		return;
	}

	KUrl::List urls = KUrl::List::fromMimeData( event->mimeData() );
	KUrl::List::const_iterator it;
	QString filename;
	
	for ( it = urls.begin(); it != urls.end(); ++it )
	{
		filename = (*it).fileName();
		if( !pdbOrPrc( filename ) )
		{
			DEBUGKPILOT << "Dropped file is not a pdb or prc file: [" << filename << ']';
			event->setAccepted(false);
			return;
		}
	}
	
	event->setAccepted(true);
}

void FileInstallWidget::dropEvent( QDropEvent * drop )
{
	FUNCTIONSETUP;
	
	if( !isVisible() )
	{
		return;
	}

	KUrl::List list = KUrl::List::fromMimeData(drop->mimeData());

	if (list.isEmpty())
	{
		return;
	}

	QStringList files;
	for(KUrl::List::ConstIterator it = list.begin(); it != list.end(); ++it)
	{
	   if ((*it).isLocalFile())
	      files << (*it).path();
	}

	fInstaller->addFiles(files, this );
}

void FileInstallWidget::slotDropEvent( QDropEvent * drop )
{
	FUNCTIONSETUP;
	dropEvent( drop );
}

void FileInstallWidget::slotClearButton()
{
	FUNCTIONSETUP;
	fInstaller->clearPending();
}

void FileInstallWidget::showPage()
{
	FUNCTIONSETUP;
	refreshFileInstallList();
}

void FileInstallWidget::slotAddFile()
{
	FUNCTIONSETUP;
	
	if (!isVisible())
	{
		return;
	}

	QStringList fileNames = KFileDialog::getOpenFileNames(
		KUrl(), i18n("*.pdb *.prc|PalmOS Databases (*.pdb *.prc)"));

	for( QStringList::Iterator fileName = fileNames.begin()
		; fileName != fileNames.end(); ++fileName )
	{
		fInstaller->addFile(*fileName, this );
	}
}

void FileInstallWidget::refreshFileInstallList()
{
	FUNCTIONSETUP;

	QStringList fileNames = fInstaller->fileNames();
	QPixmap kpilotIcon = KIconLoader::global()->loadIcon(CSL1("kpilot"), KIconLoader::Desktop);

	fP->fUi.fIconView->clear();

	for( QStringList::Iterator fileName = fileNames.begin();
		fileName != fileNames.end(); ++fileName )
	{
		if( pdbOrPrc( *fileName ) )
		{
			QListWidgetItem *item = new QListWidgetItem( *fileName, fP->fUi.fIconView );
			item->setIcon( kpilotIcon );
		}
		else
		{
			new QListWidgetItem( *fileName, fP->fUi.fIconView );
		}
	}
}

void FileInstallWidget::contextMenu( QContextMenuEvent *event )
{
	FUNCTIONSETUP;

	QListWidgetItem *item;
	QStringList files;
	int itemCount = fP->fUi.fIconView->count();
	for( int i = 0; i < itemCount; ++i )
	{
		item = fP->fUi.fIconView->item( i );
		
		if( item && item->isSelected() )
		{
			files.append( item->text() );
		}
	}

	QMenu popup( fP->fUi.fIconView );

	item = fP->fUi.fIconView->itemAt( event->pos() );
	if( item )
	{
		// Popup for the right clicked itema
		QAction *deleteItemAction =
				new QAction( i18nc( "Delete a single file item", "Delete" ), this );
		deleteItemAction->setData( (int) 10 );
		popup.addAction( deleteItemAction );
	}

	QAction *deleteAllItemsAction = new QAction( i18n( "Delete selected files" )
		, this );
	deleteAllItemsAction->setData( (int) 11 );
	popup.addAction( deleteAllItemsAction );

	if(files.empty())
	{
		deleteAllItemsAction->setEnabled( false );
	}

	QAction *action = popup.exec(
		fP->fUi.fIconView->viewport()->mapToGlobal( event->pos() ) );
	
	if( !action )
	{
		return;
	}
	else if(action->data().toInt() == 10)
	{
		fInstaller->deleteFile(item->text());
	}
	else if(action->data().toInt() == 11)
	{
		fInstaller->deleteFiles(files);
	}
}
