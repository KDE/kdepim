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

#include "printprogress.h"

#include <KApplication>
#include <KDebug>
#include <KDialog>
#include <KLocale>
#include <KTextBrowser>

#include <QGridLayout>
#include <QProgressBar>

using namespace KABPrinting;

PrintProgress::PrintProgress( QWidget *parent, const char *name )
  : QWidget( parent )
{
  setObjectName( QLatin1String(name) );
  setWindowTitle( i18n( "Printing: Progress" ) );

  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

  mLogBrowser = new KTextBrowser( this );
  topLayout->addWidget( mLogBrowser, 0, 0 );

  mProgressBar = new QProgressBar( this );
  mProgressBar->setValue( 0 );
  topLayout->addWidget( mProgressBar, 1, 0 );

  resize( QSize( 370, 220 ).expandedTo( minimumSizeHint() ) );
}

PrintProgress::~PrintProgress()
{
}

void PrintProgress::addMessage( const QString &msg )
{
  mMessages.append( msg );

  QString head = QLatin1String( "<qt><b>" ) + i18n( "Progress" ) +
                 QLatin1String( ":</b><ul>" );

  QString foot = QLatin1String( "</ul></qt>" );

  QString body;
  QStringList::ConstIterator it;
  QStringList::ConstIterator end(mMessages.constEnd());
  for ( it = mMessages.constBegin(); it != end; ++it ) {
    body.append( QLatin1String( "<li>" ) + (*it) + QLatin1String( "</li>" ) );
  }

  mLogBrowser->setText( head + body + foot );
  kapp->processEvents();
}

void PrintProgress::setProgress( int step )
{
  mProgressBar->setValue( step );
  kapp->processEvents();
}

#include "printprogress.moc"
