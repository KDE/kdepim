/* -*- C++ -*-
   This file implements the abstract print style base class.

   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2002, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Revision$
*/

#include <kstandarddirs.h>
#include <kdebug.h>

#include <qwidget.h>

#include "printstyle.h"
#include "printingwizard.h"

using namespace KABPrinting;


PrintStyle::PrintStyle( PrintingWizard* parent, const char* name )
  : QObject( parent, name ), mWizard( parent ), mSortField( 0 )
{
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
  QString path = locate( "appdata", "printing/" + fileName );
  if ( path.isEmpty() ) {
    kdDebug() << "PrintStyle::setPreview: preview not locatable." << endl;
    return false;
  } else {
    if ( preview.load( path ) ) {
      setPreview( preview );
      return true;
    } else {
      kdDebug() << "PrintStyle::setPreview: preview at '" << path << "' cannot be loaded." << endl;
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
  if ( mPageList.find( page ) == -1 ) { // not yet in the list
    mPageList.append( page );
    mPageTitles.append( title );
  }
}

void PrintStyle::showPages()
{
  QWidget *wdg = 0;
  int i = 0;
  for ( wdg = mPageList.first(); wdg; wdg = mPageList.next(), ++i ) {
    mWizard->addPage( wdg, mPageTitles[ i ] );
    if ( i == 0 )
      mWizard->setAppropriate( wdg, true );
  }

  if ( wdg )
    mWizard->setFinishEnabled( wdg, true );
}

void PrintStyle::hidePages()
{
  for ( QWidget *wdg = mPageList.first(); wdg; wdg = mPageList.next() )
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
