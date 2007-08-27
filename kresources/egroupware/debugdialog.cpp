/*
    This file is part of kdepim.
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QFile>
#include <kfiledialog.h>
#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>

#include <ktextbrowser.h>
#include <k3staticdeleter.h>

#include <stdlib.h>
#include <klocale.h>
#include "debugdialog.h"

DebugDialog* DebugDialog::mSelf = 0;
static K3StaticDeleter<DebugDialog> debugDialogDeleter;

DebugDialog::DebugDialog()
 : KDialog( 0)
{
  setCaption( i18n(  "Debug Dialog" ) );
  setButtons( User1 | User2 | Ok );
  setDefaultButton( Ok );
  setModal( false );
  showButtonSeparator( true );
  //add style Qt::WStyle_DialogBorder | Qt::WStyle_StaysOnTop

  QWidget *page = new QWidget(this);
  setMainWidget( page );
  QVBoxLayout *layout = new QVBoxLayout( page );
  layout->setSpacing( spacingHint() );
  layout->setMargin( marginHint() );

  mView = new KTextBrowser( page );
  layout->addWidget( mView );

  setButtonText( User1, "Save As..." );
  setButtonText( User2, "Clear" );
  connect(this,SIGNAL(user1Clicked()),this,SLOT(save()));
  connect(this,SIGNAL(user2Clicked()),this,SLOT(clear()));
  clear();
}

void DebugDialog::init()
{
  if ( !mSelf ) {
    if ( getenv( "EGROUPWARE_DEBUG" ) != 0 ) {
      debugDialogDeleter.setObject( mSelf, new DebugDialog );
    }
  }

  if ( mSelf ) {
    mSelf->show();
    mSelf->raise();
  }
}

DebugDialog::~DebugDialog()
{
  mSelf = 0;
}

void DebugDialog::addMessage( const QString &msg, Type type )
{
  if ( mSelf )
    mSelf->addText( msg, type );
}

void DebugDialog::clear()
{
  mView->clear();
  mMessages.clear();
}

void DebugDialog::save()
{
  QString fileName = KFileDialog::getSaveFileName();
  if ( fileName.isEmpty() )
    return;

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly ) ) {
    qWarning( "Couldn't open file %s", file.fileName().toLatin1().data() );
    return;
  }

  file.write( mMessages.join( "\n\n" ).toUtf8() );
  file.close();
}

void DebugDialog::slotUser1()
{
  save();
}

void DebugDialog::slotUser2()
{
  clear();
}

void DebugDialog::addText( const QString &text, Type type )
{
  QString htmlCode( text );
  htmlCode.replace( "<", "&lt;" );
  htmlCode.replace( ">", "&gt;" );
  htmlCode.replace( "\n", "<br>" );

  mMessages.append( text );
  if ( type == Input )
    mHTMLMessages.append( "<font color=\"green\">" + htmlCode + "</font>" );
  else
    mHTMLMessages.append( "<font color=\"blue\">" + htmlCode + "</font>" );

  mView->clear();
  mView->setHtml( mHTMLMessages.join( "<br>" ) );
}

#include "debugdialog.moc"
