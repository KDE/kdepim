/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kapplication.h>
#include <kmessagebox.h>
#include <krun.h>
#include <libkdepim/addresseeview.h>

#include "kabprefs.h"

#include "look_html.h"

KABHtmlView::KABHtmlView( QWidget *parent, const char *name )
  : KABBasicLook( parent, name )
{
  mView = new KPIM::AddresseeView( this );

  connect( mView, SIGNAL( phoneNumberClicked( const QString& ) ),
           this, SLOT( phoneNumberClicked( const QString& ) ) );

  connect( mView, SIGNAL( faxNumberClicked( const QString& ) ),
           this, SLOT( faxNumberClicked( const QString& ) ) );
}

KABHtmlView::~KABHtmlView()
{
}

void KABHtmlView::setAddressee( const KABC::Addressee &addr )
{
  mView->setAddressee( addr );
}

void KABHtmlView::phoneNumberClicked( const QString &number )
{
  QString commandLine = KABPrefs::instance()->phoneHookApplication();

  if ( commandLine.isEmpty() ) {
    KMessageBox::sorry( this, i18n( "There is no application set which could be executed. Please go to the settings dialog and configure one." ) );
    return;
  }

  commandLine.replace( "%N", number );
  KRun::runCommand( commandLine );
}

void KABHtmlView::faxNumberClicked( const QString &number )
{
  QString commandLine = KABPrefs::instance()->faxHookApplication();

  if ( commandLine.isEmpty() ) {
    KMessageBox::sorry( this, i18n( "There is no application set which could be executed. Please go to the settings dialog and configure one." ) );
    return;
  }

  commandLine.replace( "%N", number );
  KRun::runCommand( commandLine );
}

#include "look_html.moc"
