/*
    This file is part of Kung.

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

#include <tqapplication.h>
#include <tqtimer.h>
#include <tqwidget.h>

#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "generalconversationmanager.h"
#include "inputdialog.h"
#include "inputfieldfactory.h"
#include "outputdialog.h"
#include "pageinputfield.h"
#include "transport.h"

#include "dispatcher.h"

Dispatcher::Dispatcher()
  : TQObject( 0, "Dispatcher" ),
    mConversationManager( 0 )
{
}

Dispatcher::~Dispatcher()
{
  delete mConversationManager;
  mConversationManager = 0;
}

void Dispatcher::setWSDL( const KWSDL::WSDL &wsdl )
{
  mWSDL = wsdl;

  InputFieldFactory::self()->setTypes( mWSDL.types() );

  mConversationManager = new GeneralConversationManager( mWSDL );

  mTransport = new Transport( mWSDL.service().ports().first().mLocation );
  connect( mTransport, TQT_SIGNAL( result( const TQString& ) ),
           this, TQT_SLOT( result( const TQString& ) ) );
  connect( mTransport, TQT_SIGNAL( error( const TQString& ) ),
           this, TQT_SLOT( error( const TQString& ) ) );
}

void Dispatcher::run()
{
  nextMessage();
}

void Dispatcher::nextMessage()
{
  if ( !mConversationManager ) {
    qDebug( "No conversation manager set... aborting" );
    return;
  }

  TQStringList items = mConversationManager->nextActions( mLastMessage, TQString() );

  mCurrentMessage = TQString();
  if ( items.count() > 1 ) {
    mCurrentMessage = KInputDialog::getItem( i18n( "Select a functionality of the service:" ), i18n( "Functions" ),
                                         items );
  } else
    mCurrentMessage = items.first();

  if ( mCurrentMessage.isEmpty() ) {
    qApp->quit();
    return;
  }

  KWSDL::Message message = mWSDL.findMessage( mCurrentMessage );
  InputField *field = new PageInputField( message.name(), message );
  TQWidget *page = field->createWidget( 0 );

  InputDialog dlg( page, 0 );
  if ( dlg.exec() ) {
    TQDomDocument doc( "kwsdl" );
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
    TQDomElement env = doc.createElement( "SOAP-ENV:Envelope" );
    env.setAttribute( "xmlns:SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/" );
    env.setAttribute( "xmlns:xsi", "http://www.w3.org/1999/XMLSchema-instance" );
    env.setAttribute( "xmlns:xsd", "http://www.w3.org/1999/XMLSchema" );
    doc.appendChild( env );

    TQDomElement body = doc.createElement( "SOAP-ENV:Body" );
    env.appendChild( body );

    field->xmlData( doc, body );

    TQDomElement method = body.firstChild().toElement();
    TQString nameSpace = mWSDL.findBindingOperation( "", message.name() ).input().nameSpace();
    method.setAttribute( "xmlns:ns1", "urn:GoogleSearch" );
    method.setAttribute( "SOAP-ENV:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/" );
    body.appendChild( method );

    if ( mTransport ) {
      qDebug( "%s", doc.toString( 2 ).latin1() );
      mTransport->query( doc.toString( 2 ) );
    }
  } else
    qApp->quit();
}

void Dispatcher::result( const TQString &xml )
{
  qDebug( "Got data %s", xml.latin1() );

  KWSDL::Message message = mWSDL.findOutputMessage( mCurrentMessage );
  InputField *field = new PageInputField( message.name(), message );

  TQDomDocument doc;
  doc.setContent( xml, true );

  field->setXMLData( doc.documentElement().firstChild().firstChild().toElement() );
  TQWidget *page = field->createWidget( 0 );

  OutputDialog dlg( page, 0 );
  dlg.exec();

  mLastMessage = mCurrentMessage;

  TQTimer::singleShot( 0, this, TQT_SLOT( nextMessage() ) );
}

void Dispatcher::error( const TQString &errorMsg )
{
  KMessageBox::error( 0, errorMsg );

  TQTimer::singleShot( 0, this, TQT_SLOT( nextMessage() ) );
}

#include "dispatcher.moc"
