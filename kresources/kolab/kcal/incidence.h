/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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

#ifndef KOLAB_INCIDENCE_H
#define KOLAB_INCIDENCE_H

#include <kolabbase.h>

class QDomElement;

namespace KCal {
  class Incidence;
  class Recurrence;
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
    QString cycle;
    QString type;
    int interval;
    QStringList days; // list of days-of-the-week
    QString dayNumber;
    QString date;
    QString month;
    QString rangeType;
    QString range; // date or number or nothing
    QValueList<QDate> exclusions;
  };

  struct Attendee : Email {
    Attendee() : requestResponse( true ), invitationSent( false ) {}
    QString status;
    bool requestResponse;
    bool invitationSent;
    QString role;
  };

  explicit Incidence( const QString& tz, KCal::Incidence* incidence = 0 );
  virtual ~Incidence();

  void saveTo( KCal::Incidence* incidence );

  virtual void setSummary( const QString& summary );
  virtual QString summary() const;

  virtual void setLocation( const QString& location );
  virtual QString location() const;

  virtual void setOrganizer( const Email& organizer );
  virtual Email organizer() const;

  virtual void setStartDate( const QDateTime& startDate );
  virtual void setStartDate( const QDate& startDate );
  virtual void setStartDate( const QString& startDate );
  virtual QDateTime startDate() const;

  virtual void setAlarm( float alarm );
  virtual float alarm() const;

  virtual void setRecurrence( KCal::Recurrence* recur );
  virtual Recurrence recurrence() const;

  virtual void addAttendee( const Attendee& attendee );
  QValueList<Attendee>& attendees();
  const QValueList<Attendee>& attendees() const;

  // Load the attributes of this class
  virtual bool loadAttribute( QDomElement& );

  // Save the attributes of this class
  virtual bool saveAttributes( QDomElement& ) const;

protected:
  enum FloatingStatus { Unset, AllDay, HasTime };

  // Read all known fields from this ical incidence
  void setFields( const KCal::Incidence* );

  bool loadAttendeeAttribute( QDomElement&, Attendee& );
  void saveAttendeeAttribute( QDomElement& element,
                              const Attendee& attendee ) const;
  void saveAttendees( QDomElement& element ) const;

  void loadRecurrence( const QDomElement& element );
  void saveRecurrence( QDomElement& element ) const;
  void saveCustomAttributes( QDomElement& element ) const;
  void loadCustomAttributes( QDomElement& element );

  QString productID() const;

  QString mSummary;
  QString mLocation;
  Email mOrganizer;
  QDateTime mStartDate;
  FloatingStatus mFloatingStatus;
  float mAlarm;
  bool mHasAlarm;
  Recurrence mRecurrence;
  QValueList<Attendee> mAttendees;

  struct Custom {
    QCString key;
    QString value;
  };
  QValueList<Custom> mCustomList;
};

}

#endif // KOLAB_INCIDENCE_H
