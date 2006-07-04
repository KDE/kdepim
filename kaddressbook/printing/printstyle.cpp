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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kstandarddirs.h>
#include <kdebug.h>

#include <QWidget>
//Added by qt3to4:
#include <QPixmap>

#include "printstyle.h"
#include "printingwizard.h"

using namespace KABPrinting;


PrintStyle::PrintStyle( PrintingWizard* parent, const char* name )
  : QObject( parent ), mWizard( parent ), mSortField( 0 )
{
  setObjectName( name );
}

PrintStyle::~PrintStyle()
{
}

const QPixmap& PrintStyle::preview()
{
  return mPreview;
}

void PrintStyle::setPreview( const QPixmap& image )
{
  mPreview = image;
}

bool PrintStyle::setPreview( const QString& fileName )
{
  QPixmap preview;
  QString path = KStandardDirs::locate( "appdata", "printing/" + fileName );
  if ( path.isEmpty() ) {
    kDebug(5720) << "PrintStyle::setPreview: preview not locatable." << endl;
    return false;
  } else {
    if ( preview.load( path ) ) {
      setPreview( preview );
      return true;
    } else {
      kDebug(5720) << "PrintStyle::setPreview: preview at '" << path << "' cannot be loaded." << endl;
      return false;
    }
  }
}

PrintingWizard *PrintStyle::wizard()
{
  return mWizard;
}

void PrintStyle::addPage( QWidget *page, const QString &title )
{
  if ( mPageList.indexOf( page ) == -1 ) { // not yet in the list
    mPageList.append( page );
    mPageTitles.append( title );
  }
}

void PrintStyle::showPages()
{
  QWidget *wdg = 0;
  int i = 0;
  Q_FOREACH( wdg, mPageList ) {
    mWizard->addPage( wdg, mPageTitles[ i ] );
    if ( i == 0 )
      mWizard->setAppropriate( wdg, true );
  }

  if ( wdg )
    mWizard->setFinishEnabled( wdg, true );
}

void PrintStyle::hidePages()
{

  Q_FOREACH( QWidget *wdg, mPageList )
    mWizard->removePage( wdg );
}

void PrintStyle::setPreferredSortOptions( KABC::Field *field, bool ascending )
{
  mSortField = field;
  mSortType = ascending;
}

KABC::Field* PrintStyle::preferredSortField()
{
  return mSortField;
}

bool PrintStyle::preferredSortType()
{
  return mSortType;
}

PrintStyleFactory::PrintStyleFactory( PrintingWizard* parent, const char* name )
        : mParent( parent ), mName( name )
{
}

PrintStyleFactory::~PrintStyleFactory()
{
}

#include "printstyle.moc"
