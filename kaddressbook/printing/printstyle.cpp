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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
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
    kdDebug(5720) << "PrintStyle::setPreview: preview not locatable." << endl;
    return false;
  } else {
    if ( preview.load( path ) ) {
      setPreview( preview );
      return true;
    } else {
      kdDebug(5720) << "PrintStyle::setPreview: preview at '" << path << "' cannot be loaded." << endl;
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
