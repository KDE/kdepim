/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2011 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filteractionmissingcollectiondialog.h"

#include "folderrequester.h"

#include <KLocale>

#include <QVBoxLayout>
#include <QLabel>


FilterActionMissingCollectionDialog::FilterActionMissingCollectionDialog( const QString& filtername, QWidget *parent )
  : KDialog( parent )
{
  setModal( true );
  setCaption( i18n( "Select Folder" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  QVBoxLayout* lay = new QVBoxLayout( mainWidget() );
  QLabel *label = new QLabel( this );
  label->setText( i18n( "Folder is missing. Select a folder for filter \"%1\" please", filtername ) );
  lay->addWidget( label );
  mFolderRequester = new MailCommon::FolderRequester( this );
  lay->addWidget( mFolderRequester );
}

FilterActionMissingCollectionDialog::~FilterActionMissingCollectionDialog()
{
}

Akonadi::Collection FilterActionMissingCollectionDialog::selectedCollection() const
{
  return mFolderRequester->collection();
}



#include "filteractionmissingcollectiondialog.moc"

