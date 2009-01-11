/*
  This file is part of libkabc.
  Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2008 Kevin Krammer <kevin.krammer@gmx.at>

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

#include "contactgrouptool.h"
#include "contactgroup.h"

#include <QtCore/QIODevice>
#include <QtCore/QString>
#include <QtCore/QDebug>

#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

using namespace KPIM;

class XmlContactGroupWriter : public QXmlStreamWriter
{
  public:
    XmlContactGroupWriter();

    void write( const ContactGroup &group, QIODevice *device );
    void write( const QList<ContactGroup> &groupLis, QIODevice *device );

  private:
    void writeGroup( const ContactGroup &group );
    void writeReference( const ContactGroup::Reference & );
    void writeData( const ContactGroup::Data & );
};

XmlContactGroupWriter::XmlContactGroupWriter()
{
  setAutoFormatting( true );
}

void XmlContactGroupWriter::write( const ContactGroup &group, QIODevice *device )
{
  setDevice( device );

  writeStartDocument();

  writeGroup( group );

  writeEndDocument();
}

void XmlContactGroupWriter::write( const QList<ContactGroup> &groupList, QIODevice *device )
{
  setDevice( device );

  writeStartDocument();

  writeStartElement( "contactGroupList" );

  foreach ( const ContactGroup & group, groupList ) {
    writeGroup( group );
  }

  writeEndElement();

  writeEndDocument();
}

void XmlContactGroupWriter::writeGroup( const ContactGroup &group )
{
  writeStartElement( "contactGroup" );
  writeAttribute( "uid", group.id() );
  writeAttribute( "name", group.name() );

  for ( uint i = 0; i < group.referencesCount(); ++i ) {
    writeReference( group.reference( i ) );
  }

  for ( uint i = 0; i < group.dataCount(); ++i ) {
    writeData( group.data( i ) );
  }

  writeEndElement();
}

void XmlContactGroupWriter::writeReference( const ContactGroup::Reference &reference )
{
  writeStartElement( "contactReference" );
  writeAttribute( "uid", reference.uid() );
  if ( !reference.preferredEmail().isEmpty() ) {
    writeAttribute( "preferredEmail", reference.preferredEmail() );
  }

  // TODO: customs

  writeEndElement();
}

void XmlContactGroupWriter::writeData( const ContactGroup::Data &data )
{
  writeStartElement( "contactData" );
  writeAttribute( "name", data.name() );
  writeAttribute( "email", data.email() );

  // TODO: customs

  writeEndElement();
}

class XmlContactGroupReader : public QXmlStreamReader
{
  public:
    XmlContactGroupReader();

    bool read( QIODevice *device, ContactGroup &group );
    bool read( QIODevice *device, QList<ContactGroup> &groupList );

  private:
    bool readGroup( ContactGroup &group );
    bool readReference( ContactGroup::Reference &reference );
    bool readData( ContactGroup::Data &data );
};

XmlContactGroupReader::XmlContactGroupReader()
{
}

bool XmlContactGroupReader::read( QIODevice *device, ContactGroup &group )
{
  setDevice( device );

  while ( !atEnd() ) {
    readNext();
    if ( isStartElement() ) {
      if ( name() == QLatin1String( "contactGroup" ) ) {
        return readGroup( group );
      } else {
        raiseError( "The document does not describe a ContactGroup" );
      }
    }
  }

  return error() == NoError;
}

bool XmlContactGroupReader::read( QIODevice *device, QList<ContactGroup> &groupList )
{
  setDevice( device );

  int depth = 0;

  while ( !atEnd() ) {
    readNext();
    if ( isStartElement() ) {
      ++depth;
      if ( depth == 1 ) {
        if ( name() == QLatin1String( "contactGroupList" ) ) {
          continue;
        } else {
          raiseError( "The document does not describe a list of ContactGroup" );
        }
      } else if ( depth == 2 ) {
        if ( name() == QLatin1String( "contactGroup" ) ) {
          ContactGroup group;
          if ( !readGroup( group ) ) {
            return false;
          }

          groupList.append( group );
        } else {
          raiseError( "The document does not describe a list of ContactGroup" );
        }
      }
    }

    if ( isEndElement() ) {
      --depth;
    }
  }

  return error() == NoError;
}

bool XmlContactGroupReader::readGroup( ContactGroup &group )
{
  const QXmlStreamAttributes elementAttributes = attributes();
  const QStringRef uid = elementAttributes.value( "uid" );
  if ( uid.isEmpty() ) {
    raiseError( "ContactGroup is missing a uid" );
    return false;
  }

  const QStringRef groupName = elementAttributes.value( "name" );
  if ( groupName.isEmpty() ) {
    raiseError( "ContactGroup is missing a name" );
    return false;
  }

  group.setId( uid.toString() );
  group.setName( groupName.toString() );

  while ( !atEnd() ) {
    readNext();
    if ( isStartElement() ) {
      if ( name() == QLatin1String( "contactData" ) ) {
        ContactGroup::Data data;
        if ( !readData( data ) ) {
          return false;
        }
        group.append( data );
      } else if ( name() == QLatin1String( "contactReference" ) ) {
        ContactGroup::Reference reference;
        if ( !readReference( reference ) ) {
          return false;
        }
        group.append( reference );
      } else {
        raiseError( "The document does not describe a ContactGroup" );
      }
    }

    if ( isEndElement() ) {
      if ( name() == QLatin1String( "contactGroup" ) ) {
        return true;
      }
    }
  }

  return false;
}

bool XmlContactGroupReader::readData( ContactGroup::Data &data )
{
  const QXmlStreamAttributes elementAttributes = attributes();
  const QStringRef email = elementAttributes.value( "email" );
  if ( email.isEmpty() ) {
    raiseError( "ContactData is missing an email address" );
    return false;
  }

  const QStringRef name = elementAttributes.value( "name" );
  if ( name.isEmpty() ) {
    raiseError( "ContactData is missing a name" );
    return false;
  }

  data.setName( name.toString() );
  data.setEmail( email.toString() );

  return true;
}

bool XmlContactGroupReader::readReference( ContactGroup::Reference &reference )
{
  const QXmlStreamAttributes elementAttributes = attributes();
  const QStringRef uid = elementAttributes.value( "uid" );
  if ( uid.isEmpty() ) {
    raiseError( "ContactReference is missing a uid" );
    return false;
  }

  reference.setUid( uid.toString() );

  return true;
}

bool ContactGroupTool::convertFromXml( QIODevice *device, ContactGroup &group,
                                       QString *errorMessage )
{
  Q_UNUSED( errorMessage );

  XmlContactGroupReader reader;

  bool ok = reader.read( device, group );

  if ( !ok && errorMessage != 0 ) {
    *errorMessage = reader.errorString();
  }

  return ok;
}

bool ContactGroupTool::convertToXml( const ContactGroup &group, QIODevice *device,
                                     QString *errorMessage )
{
  Q_UNUSED( errorMessage );

  XmlContactGroupWriter writer;
  writer.write( group, device );

  return true;
}

bool ContactGroupTool::convertFromXml( QIODevice *device, QList<ContactGroup> &groupList,
                                       QString *errorMessage )
{
  Q_UNUSED( errorMessage );

  XmlContactGroupReader reader;

  bool ok = reader.read( device, groupList );

  if ( !ok && errorMessage != 0 ) {
    *errorMessage = reader.errorString();
  }

  return ok;
}

bool ContactGroupTool::convertToXml( const QList<ContactGroup> &groupList,
                                     QIODevice *device, QString *errorMessage )
{
  Q_UNUSED( errorMessage );

  XmlContactGroupWriter writer;
  writer.write( groupList, device );

  return true;
}
