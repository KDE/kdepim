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

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <kmimemagic.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <ktempfile.h>

#include <tqfile.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqwidget.h>

#include <schema/simpletype.h>

#include "binaryinputfield.h"

BinaryInputField::BinaryInputField( const TQString &name, const TQString &typeName, const Schema::SimpleType *type )
  : SimpleInputField( name, type ),
    mValue( 0 ), mTypeName( typeName )
{
}

void BinaryInputField::setXMLData( const TQDomElement &element )
{
  if ( mName != element.tagName() ) {
    qDebug( "BinaryInputField: Wrong dom element passed: expected %s, got %s", mName.latin1(), element.tagName().latin1() );
    return;
  }

  setData( element.text() );
}

void BinaryInputField::xmlData( TQDomDocument &document, TQDomElement &parent )
{
  TQDomElement element = document.createElement( mName );
  element.setAttribute( "xsi:type", "xsd:" + mTypeName );
  TQDomText text = document.createTextNode( data() );
  element.appendChild( text );

  parent.appendChild( element );
}

void BinaryInputField::setData( const TQString &data )
{
  KCodecs::base64Decode( data.utf8(), mValue );
}

TQString BinaryInputField::data() const
{
  TQByteArray data = KCodecs::base64Encode( mValue, false );
  return TQString::fromUtf8( data.data(), data.size() );
}

TQWidget *BinaryInputField::createWidget( TQWidget *parent )
{
  mInputWidget = new BinaryWidget( parent );

  if ( !mValue.isEmpty() )
    mInputWidget->setData( mValue );

  return mInputWidget;
}

void BinaryInputField::valueChanged( const TQByteArray &value )
{
  mValue = value;

  emit modified();
}


BinaryWidget::BinaryWidget( TQWidget *parent )
  : TQWidget( parent, "BinaryWidget" ),
    mMainWidget( 0 )
{
  mLayout = new TQGridLayout( this, 3, 2, 11, 6 );

  mLoadButton = new TQPushButton( i18n( "Load..." ), this );
  mSaveButton = new TQPushButton( i18n( "Save..." ), this );
  mSaveButton->setEnabled( false );

  mLayout->addWidget( mLoadButton, 0, 1 );
  mLayout->addWidget( mSaveButton, 1, 1 );

  connect( mLoadButton, TQT_SIGNAL( clicked() ), TQT_SLOT( load() ) );
  connect( mSaveButton, TQT_SIGNAL( clicked() ), TQT_SLOT( save() ) );
}

void BinaryWidget::setData( const TQByteArray &data )
{
  KMimeMagic magic;
  TQString mimetype;

  delete mMainWidget;

  KMimeMagicResult *result = magic.findBufferType( data );
  if ( result->isValid() )
    mimetype = result->mimeType();

  if ( !mimetype.isEmpty() ) {
    KParts::ReadOnlyPart *part = KParts::ComponentFactory::createPartInstanceFromQuery<KParts::ReadOnlyPart>( mimetype, TQString::null, this, 0, this, 0 );
    if ( part ) {
      KTempFile file;
      file.file()->writeBlock( data );
      file.close();
      part->openURL( KURL( file.name() ) );
      mMainWidget = part->widget();
    } else {
      mMainWidget = new TQLabel( i18n( "No part found for visualization of mimetype %1" ).arg( mimetype ), this );
    }

    mData = data;
    mSaveButton->setEnabled( true );
  } else {
    mMainWidget = new TQLabel( i18n( "Got data of unknown mimetype" ), this );
  }

  mLayout->addMultiCellWidget( mMainWidget, 0, 2, 0, 0 );
  mMainWidget->show();
}

void BinaryWidget::load()
{
  KURL url = KFileDialog::getOpenURL( TQString(), TQString(), this );
  if ( url.isEmpty() )
    return;

  TQString tempFile;
  if ( KIO::NetAccess::download( url, tempFile, this ) ) {
    TQFile file( tempFile );
    if ( !file.open( IO_ReadOnly ) ) {
      KMessageBox::error( this, i18n( "Unable to open file %1" ).arg( url.url() ) );
      return;
    }

    TQByteArray data = file.readAll();
    setData( data );

    file.close();
    KIO::NetAccess::removeTempFile( tempFile );

    emit valueChanged( data );
  } else
    KMessageBox::error( this, KIO::NetAccess::lastErrorString() );
}

void BinaryWidget::save()
{
  KURL url = KFileDialog::getSaveURL( TQString(), TQString(), this );
  if ( url.isEmpty() )
    return;

  KTempFile tempFile;
  tempFile.file()->writeBlock( mData );
  tempFile.close();

  if ( !KIO::NetAccess::upload( tempFile.name(), url, this ) )
    KMessageBox::error( this, KIO::NetAccess::lastErrorString() );
}

#include "binaryinputfield.moc"
