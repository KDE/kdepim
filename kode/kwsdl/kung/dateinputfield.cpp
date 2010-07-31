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

#include <kdatepicker.h>

#include "dateinputfield.h"

DateInputField::DateInputField( const TQString &name, const Schema::SimpleType *type )
  : SimpleInputField( name, type ),
    mValue( TQDate::currentDate() )
{
}

void DateInputField::setXMLData( const TQDomElement &element )
{
  if ( mName != element.tagName() ) {
    qDebug( "DateInputField: Wrong dom element passed: expected %s, got %s", mName.latin1(), element.tagName().latin1() );
    return;
  }

  setData( element.text() );
}

void DateInputField::xmlData( TQDomDocument &document, TQDomElement &parent )
{
  TQDomElement element = document.createElement( mName );
  element.setAttribute( "xsi:type", "xsd:date" );
  TQDomText text = document.createTextNode( data() );
  element.appendChild( text );

  parent.appendChild( element );
}

void DateInputField::setData( const TQString &data )
{
  mValue = TQDate::fromString( data, Qt::ISODate );
}

TQString DateInputField::data() const
{
  return mValue.toString( Qt::ISODate );
}

TQWidget *DateInputField::createWidget( TQWidget *parent )
{
  mInputWidget = new KDatePicker( parent );

  mInputWidget->setDate( mValue );

  connect( mInputWidget, TQT_SIGNAL( dateChanged( TQDate ) ),
           this, TQT_SLOT( inputChanged( TQDate ) ) );

  return mInputWidget;
}

void DateInputField::inputChanged( TQDate date )
{
  mValue = date;

  emit modified();
}

#include "dateinputfield.moc"
