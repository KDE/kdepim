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

#include <knuminput.h>

#include <schema/simpletype.h>

#include <limits.h>

#include "integerinputfield.h"

IntegerInputField::IntegerInputField( const TQString &name, const TQString &typeName, const Schema::SimpleType *type )
  : SimpleInputField( name, type ),
    mValue( 0 ), mTypeName( typeName )
{
}

void IntegerInputField::setXMLData( const TQDomElement &element )
{
  if ( mName != element.tagName() ) {
    qDebug( "IntegerInputField: Wrong dom element passed: expected %s, got %s", mName.latin1(), element.tagName().latin1() );
    return;
  }

  setData( element.text() );
}

void IntegerInputField::xmlData( TQDomDocument &document, TQDomElement &parent )
{
  TQDomElement element = document.createElement( mName );
  element.setAttribute( "xsi:type", "xsd:" + mTypeName );
  TQDomText text = document.createTextNode( data() );
  element.appendChild( text );

  parent.appendChild( element );
}

void IntegerInputField::setData( const TQString &data )
{
  mValue = data.toInt();
}

TQString IntegerInputField::data() const
{
  return TQString::number( mValue );
}

TQWidget *IntegerInputField::createWidget( TQWidget *parent )
{
  mInputWidget = new KIntSpinBox( parent );

  // basic restrictions
  if ( mTypeName == "byte" ) {
    mInputWidget->setMinValue( CHAR_MIN );
    mInputWidget->setMaxValue( CHAR_MAX );
  } else if ( mTypeName == "unsignedByte" ) {
    mInputWidget->setMinValue( 0 );
    mInputWidget->setMaxValue( UCHAR_MAX );
  } else if ( mTypeName == "integer" || mTypeName == "int" ) {
    mInputWidget->setMinValue( INT_MIN );
    mInputWidget->setMaxValue( INT_MAX );
  } else if ( mTypeName == "positiveInteger" ) {
    mInputWidget->setMinValue( 1 );
    mInputWidget->setMaxValue( UINT_MAX );
  } else if ( mTypeName == "negativeInteger" ) {
    mInputWidget->setMinValue( INT_MIN );
    mInputWidget->setMaxValue( -1 );
  } else if ( mTypeName == "nonNegativeInteger" || mTypeName == "unsignedInt" ) {
    mInputWidget->setMinValue( 0 );
    mInputWidget->setMaxValue( UINT_MAX );
  } else if ( mTypeName == "nonPositiveInteger" ) {
    mInputWidget->setMinValue( INT_MIN );
    mInputWidget->setMaxValue( 0 );
  } else if ( mTypeName == "long" ) {
    mInputWidget->setMinValue( LONG_MIN );
    mInputWidget->setMaxValue( LONG_MAX );
  } else if ( mTypeName == "unsignedlong" ) {
    mInputWidget->setMinValue( 0 );
    mInputWidget->setMaxValue( ULONG_MAX );
  } else if ( mTypeName == "short" ) {
    mInputWidget->setMinValue( SHRT_MIN );
    mInputWidget->setMaxValue( SHRT_MAX );
  } else if ( mTypeName == "unsignedShort" ) {
    mInputWidget->setMinValue( 0 );
    mInputWidget->setMaxValue( USHRT_MAX );
  }

  if ( mType ) {
    if ( mType->facetType() & Schema::SimpleType::MININC )
      mInputWidget->setMinValue( mType->facetMinimumInclusive() );
    if ( mType->facetType() & Schema::SimpleType::MINEX )
      mInputWidget->setMinValue( mType->facetMinimumExclusive() + 1 );
    if ( mType->facetType() & Schema::SimpleType::MAXINC )
      mInputWidget->setMaxValue( mType->facetMaximumInclusive() );
    if ( mType->facetType() & Schema::SimpleType::MAXEX )
      mInputWidget->setMaxValue( mType->facetMaximumExclusive() - 1 );
  }

  mInputWidget->setValue( mValue );

  connect( mInputWidget, TQT_SIGNAL( valueChanged( int ) ),
           this, TQT_SLOT( inputChanged( int ) ) );

  return mInputWidget;
}

void IntegerInputField::inputChanged( int value )
{
  mValue = value;

  emit modified();
}

#include "integerinputfield.moc"
