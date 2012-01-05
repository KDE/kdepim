/*
  This file is part of KAddressBook.
  Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "printstyle.h"
#include "printingwizard.h"

#include <KDebug>
#include <KStandardDirs>

#include <QtGui/QPixmap>
#include <QtGui/QWidget>

using namespace KABPrinting;

PrintStyle::PrintStyle( PrintingWizard *parent )
  : QObject( parent ), mWizard( parent )
{
  mSortField = ContactFields::GivenName;
  mSortOrder = Qt::AscendingOrder;
}

PrintStyle::~PrintStyle()
{
}

const QPixmap &PrintStyle::preview() const
{
  return mPreview;
}

void PrintStyle::setPreview( const QPixmap &image )
{
  mPreview = image;
}

bool PrintStyle::setPreview( const QString &fileName )
{
  QPixmap preview;

  const QString path = KStandardDirs::locate( "data", "kaddressbook/printing/" + fileName );
  if ( path.isEmpty() ) {
    kDebug() << "cannot locate preview image " << fileName << " in appdata";
    return false;
  } else {
    if ( preview.load( path ) ) {
      setPreview( preview );
      return true;
    } else {
      kDebug() << "preview at '" << path <<"' cannot be loaded.";
      return false;
    }
  }
}

PrintingWizard *PrintStyle::wizard() const
{
  return mWizard;
}

void PrintStyle::addPage( QWidget *page, const QString &title )
{
  if ( mPageList.indexOf( page ) == -1 ) { // not yet in the list
    mPageList.append( page );
    mPageTitles.append( title );

    KPageWidgetItem *item = new KPageWidgetItem( page, title );
    mPageItems.insert( page, item );
    mWizard->addPage( item );
    mWizard->setAppropriate( item, false );
  }
}

void PrintStyle::showPages()
{
  QWidget *wdg = 0;

  Q_FOREACH ( wdg, mPageList ) {
    mWizard->setAppropriate( mPageItems[ wdg ], true );
  }

  mWizard->enableButton( KDialog::User2, wdg ); // next button
  mWizard->enableButton( KDialog::User1, !wdg ); // finish button
}

void PrintStyle::hidePages()
{
  Q_FOREACH ( QWidget *wdg, mPageList ) {
    mWizard->setAppropriate( mPageItems[ wdg ], false );
  }
}

void PrintStyle::setPreferredSortOptions( ContactFields::Field field, Qt::SortOrder sortOrder )
{
  mSortField = field;
  mSortOrder = sortOrder;
}

ContactFields::Field PrintStyle::preferredSortField() const
{
  return mSortField;
}

Qt::SortOrder PrintStyle::preferredSortOrder() const
{
  return mSortOrder;
}

PrintStyleFactory::PrintStyleFactory( PrintingWizard *parent )
  : mParent( parent )
{
}

PrintStyleFactory::~PrintStyleFactory()
{
}

#include "printstyle.moc"
