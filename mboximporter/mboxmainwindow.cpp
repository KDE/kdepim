/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mboxmainwindow.h"
#include "mboximportwidget.h"
#include "mboximportkernel.h"

#include <mailcommon/kernel/mailkernel.h>

#include <KLocale>

MBoxMainWindow::MBoxMainWindow(const QString &filename, QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Import mbox file" ) );
    setButtons( Cancel );

    setDefaultButton( Cancel );

    MBoxImporterKernel *kernel = new MBoxImporterKernel( this );
    CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

    mImportWidget = new MBoxImportWidget;
    connect(mImportWidget, SIGNAL(importMailsClicked()), this, SLOT(slotImportMBox()));
    setMainWidget( mImportWidget );
    resize( 800, 600 );
}

MBoxMainWindow::~MBoxMainWindow()
{

}

void MBoxMainWindow::slotImportMBox()
{
    //TODO
}

#include "mboxmainwindow.moc"
