/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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

#ifndef KOLABBASE_H
#define KOLABBASE_H

#include <tqdom.h>
#include <tqdatetime.h>
#include <tqcolor.h>

class QFile;

namespace KCal {
  class Incidence;
}

namespace KABC {
  class Addressee;
}

namespace Kolab {

class KolabBase {
public:
  struct Email {
  public:
    Email( const TQString& name = TQString::null,
           const TQString& email = TQString::null )
      : displayName( name ), smtpAddress( email )
    {
    }

    TQString displayName;
    TQString smtpAddress;
  };

  enum Sensitivity { Public = 0, Private = 1, Confidential = 2 };

  explicit KolabBase( const TQString& timezone = TQString::null );
  virtual ~KolabBase();

  // Return a string identifying this type
  virtual TQString type() const = 0;

  virtual void setUid( const TQString& uid );
  virtual TQString uid() const;

  virtual void setBody( const TQString& body );
  virtual TQString body() const;

  virtual void setCategories( const TQString& categories );
  virtual TQString categories() const;

  virtual void setCreationDate( const TQDateTime& date );
  virtual TQDateTime creationDate() const;

  virtual void setLastModified( const TQDateTime& date );
  virtual TQDateTime lastModified() const;

  virtual void setSensitivity( Sensitivity sensitivity );
  virtual Sensitivity sensitivity() const;

  virtual void setPilotSyncId( unsigned long id );
  virtual bool hasPilotSyncId() const;
  virtual unsigned long pilotSyncId() const;

  virtual void setPilotSyncStatus( int status );
  virtual bool hasPilotSyncStatus() const;
  virtual int pilotSyncStatus() const;

  // String - Date conversion methods
  static TQString dateTimeToString( const TQDateTime& time );
  static TQString dateToString( const TQDate& date );
  static TQDateTime stringToDateTime( const TQString& time );
  static TQDate stringToDate( const TQString& date );

  // String - Sensitivity conversion methods
  static TQString sensitivityToString( Sensitivity );
  static Sensitivity stringToSensitivity( const TQString& );

  // String - Color conversion methods
  static TQString colorToString( const TQColor& );
  static TQColor stringToColor( const TQString& );

  // Load this object by reading the XML file
  bool load( const TQString& xml );
  bool load( TQFile& xml );

  // Load this QDomDocument
  virtual bool loadXML( const TQDomDocument& xml ) = 0;

  // Serialize this object to an XML string
  virtual TQString saveXML() const = 0;

protected:
  /// Read all known fields from this ical incidence
  void setFields( const KCal::Incidence* );

  /// Save all known fields into this ical incidence
  void saveTo( KCal::Incidence* ) const;

  /// Read all known fields from this contact
  void setFields( const KABC::Addressee* );

  /// Save all known fields into this contact
  void saveTo( KABC::Addressee* ) const;

  // This just makes the initial dom tree with version and doctype
  static TQDomDocument domTree();

  bool loadEmailAttribute( TQDomElement& element, Email& email );

  void saveEmailAttribute( TQDomElement& element, const Email& email,
                           const TQString& tagName = "email" ) const;

  // Load the attributes of this class
  virtual bool loadAttribute( TQDomElement& );

  // Save the attributes of this class
  virtual bool saveAttributes( TQDomElement& ) const;

  // Return the product ID
  virtual TQString productID() const = 0;

  // Write a string tag
  static void writeString( TQDomElement&, const TQString&, const TQString& );

  TQDateTime localToUTC( const TQDateTime& time ) const;
  TQDateTime utcToLocal( const TQDateTime& time ) const;

  TQString mUid;
  TQString mBody;
  TQString mCategories;
  TQDateTime mCreationDate;
  TQDateTime mLastModified;
  Sensitivity mSensitivity;
  TQString mTimeZoneId;

  // KPilot synchronization stuff
  bool mHasPilotSyncId,  mHasPilotSyncStatus;
  unsigned long mPilotSyncId;
  int mPilotSyncStatus;
};

}

#endif // KOLABBASE_H
