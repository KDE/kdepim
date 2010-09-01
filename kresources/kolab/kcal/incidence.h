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

#ifndef KOLAB_INCIDENCE_H
#define KOLAB_INCIDENCE_H

#include <kolabbase.h>

class TQDomElement;

namespace KCal {
  class Incidence;
  class Recurrence;
  class Alarm;
  class Attachment;
  class ResourceKolab;
}

namespace Kolab {

/**
 * This abstract class represents an incidence which has the shared
 * fields, of events and tasks and knows how to load/save these
 * from/to XML, and from/to a KCal::Incidence.
 */
class Incidence : public KolabBase {
public:
  struct Recurrence {
    TQString cycle;
    TQString type;
    int interval;
    TQStringList days; // list of days-of-the-week
    TQString dayNumber;
    TQString month;
    TQString rangeType;
    TQString range; // date or number or nothing
    TQValueList<TQDate> exclusions;
  };

  struct Attendee : Email {
    Attendee() : requestResponse( true ), invitationSent( false ) {}
    TQString status;
    bool requestResponse;
    bool invitationSent;
    TQString role;
    TQString delegate;
    TQString delegator;
  };

  explicit Incidence( KCal::ResourceKolab *res, const TQString &subResource, Q_UINT32 sernum,
                      const TQString& tz );
  virtual ~Incidence();

  void saveTo( KCal::Incidence* incidence );

  virtual void setSummary( const TQString& summary );
  virtual TQString summary() const;

  virtual void setLocation( const TQString& location );
  virtual TQString location() const;

  virtual void setOrganizer( const Email& organizer );
  virtual Email organizer() const;

  virtual void setStartDate( const TQDateTime& startDate );
  virtual void setStartDate( const TQDate& startDate );
  virtual void setStartDate( const TQString& startDate );
  virtual TQDateTime startDate() const;

  virtual void setAlarm( float alarm );
  virtual float alarm() const;

  virtual void setRecurrence( KCal::Recurrence* recur );
  virtual Recurrence recurrence() const;

  virtual void addAttendee( const Attendee& attendee );
  TQValueList<Attendee>& attendees();
  const TQValueList<Attendee>& attendees() const;

  /**
   * The internal uid is used as the uid inside KOrganizer whenever
   * two or more events with the same uid appear, which KOrganizer
   * can't handle. To avoid keep that interal uid from changing all the
   * time, it is persisted in the XML between a save and the next load.
   */
  void setInternalUID( const TQString& iuid );
  TQString internalUID() const;

  // Load the attributes of this class
  virtual bool loadAttribute( TQDomElement& );

  // Save the attributes of this class
  virtual bool saveAttributes( TQDomElement& ) const;

protected:
  enum FloatingStatus { Unset, AllDay, HasTime };

  // Read all known fields from this ical incidence
  void setFields( const KCal::Incidence* );

  bool loadAttendeeAttribute( TQDomElement&, Attendee& );
  void saveAttendeeAttribute( TQDomElement& element,
                              const Attendee& attendee ) const;
  void saveAttendees( TQDomElement& element ) const;
  void saveAttachments( TQDomElement& element ) const;

  void loadAlarms( const TQDomElement& element );
  void saveAlarms( TQDomElement& element ) const;

  void loadRecurrence( const TQDomElement& element );
  void saveRecurrence( TQDomElement& element ) const;
  void saveCustomAttributes( TQDomElement& element ) const;
  void loadCustomAttributes( TQDomElement& element );

  void loadAttachments();

  TQString productID() const;

  TQString mSummary;
  TQString mLocation;
  Email mOrganizer;
  TQDateTime mStartDate;
  FloatingStatus mFloatingStatus;
  float mAlarm;
  bool mHasAlarm;
  Recurrence mRecurrence;
  TQValueList<Attendee> mAttendees;
  TQValueList<KCal::Alarm*> mAlarms;
  TQValueList<KCal::Attachment*> mAttachments;
  TQString mInternalUID;

  struct Custom {
    TQCString key;
    TQString value;
  };
  TQValueList<Custom> mCustomList;

  KCal::ResourceKolab *mResource;
  TQString mSubResource;
  Q_UINT32 mSernum;
};

}

#endif // KOLAB_INCIDENCE_H
