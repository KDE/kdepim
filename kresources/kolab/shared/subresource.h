/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef SUBRESOURCE_H
#define SUBRESOURCE_H

#include <qstring.h>
#include <qmap.h>


namespace Kolab {

/**
 * This class is used to store in a map from resource id to this, providing
 * a lookup of the subresource settings.
 */
class SubResource {
public:
  // This is just for QMap
  SubResource() {}

  SubResource( bool active, bool writable, const QString& label,
               int completionWeight = 100 );
  virtual ~SubResource();

  virtual void setActive( bool active );
  virtual bool active() const;

  virtual void setWritable( bool writable );
  virtual bool writable() const;

  virtual void setLabel( const QString& label );
  virtual QString label() const;

  virtual void setCompletionWeight( int completionWeight );
  virtual int completionWeight() const;

private:
  bool mActive;   // Controlled by the applications
  bool mWritable; // Set if the KMail folder is writable
  QString mLabel; // The GUI name of this resource

  // This is just for the abc plugin. But as long as only this is here,
  // it's just as cheap to have it in here as making a d-pointer that
  // subclasses could add to. If more are added, then we should refactor
  // to a d-pointer instead.
  int mCompletionWeight;
};

typedef QMap<QString, SubResource> ResourceMap;

/**
 * This class is used to store a mapping from the XML UID to the KMail
 * serial number of the mail it's stored in and the resource. That provides
 * a quick way to access the storage in KMail.
 */
class StorageReference {
public:
  // Just for QMap
  StorageReference() {}

  StorageReference( const QString& resource, Q_UINT32 sernum );
  virtual ~StorageReference();

  virtual void setResource( const QString& resource );
  virtual QString resource() const;

  virtual void setSerialNumber( Q_UINT32 serialNumber );
  virtual Q_UINT32 serialNumber() const;

private:
  QString mResource;
  Q_UINT32 mSerialNumber;
};

typedef QMap<QString, StorageReference> UidMap;

}

#endif // SUBRESOURCE_H
