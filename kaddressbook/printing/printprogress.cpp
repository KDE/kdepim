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

#include <tqlayout.h>
#include <tqprogressbar.h>
#include <tqtextbrowser.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>

#include "printprogress.h"

using namespace KABPrinting;

PrintProgress::PrintProgress( TQWidget *parent, const char *name )
  : TQWidget( parent, name )
{
  setCaption( i18n( "Printing: Progress" ) );

  TQGridLayout *topLayout = new TQGridLayout( this, 1, 1, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  mLogBrowser = new TQTextBrowser( this );
  topLayout->addWidget( mLogBrowser, 0, 0 );

  mProgressBar = new TQProgressBar( this );
  mProgressBar->setProgress( 0 );
  topLayout->addWidget( mProgressBar, 1, 0 );

  resize( TQSize( 370, 220 ).expandedTo( minimumSizeHint() ) );
}

PrintProgress::~PrintProgress()
{
}

void PrintProgress::addMessage( const TQString &msg )
{
  mMessages.append( msg );

  TQString head = TQString( "<qt><b>" ) + i18n( "Progress" ) +
                 TQString( ":</b><ul>" );

  TQString foot = TQString( "</ul></qt>" );

  TQString body;
  TQStringList::ConstIterator it;
  for ( it = mMessages.begin(); it != mMessages.end(); ++it )
    body.append( TQString( "<li>" ) + (*it) + TQString( "</li>" ) );

  mLogBrowser->setText( head + body + foot );
  kapp->processEvents();
}

void PrintProgress::setProgress( int step )
{
  mProgressBar->setProgress( step );
  kapp->processEvents();
}

#include "printprogress.moc"
