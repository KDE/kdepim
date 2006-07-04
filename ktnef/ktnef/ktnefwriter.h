/*
    ktnefwriter.cpp

    Copyright (C) 2002 Bo Thorsen  <bo@sonofthor.dk>

    This file is part of KTNEF, the KDE TNEF support library/program.

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

#ifndef KTNEFWRITER_H
#define KTNEFWRITER_H

class QString;
class QVariant;
class QIODevice;
class QDataStream;
class QDateTime;
class QStringList;

class KTNEFWriter {
public:
  KTNEFWriter();
  ~KTNEFWriter();

  void addProperty( int tag, int type, const QVariant& value );

  bool writeFile( QIODevice &file );
  bool writeFile( QDataStream &stream );

  bool writeProperty( QDataStream &stream, int &bytes, int type);

  enum MessageType {
    Appointment, MeetingCancelled, MeetingRequest,
    MeetingNo, MeetingYes, MeetingTent
  };

  enum Method {
      PublishNew, Obsolete, RequestNew, RequestUpdate, Unknown
  };

  enum Role {
    ReqParticipant, OptParticipant, NonParticipant, Chair
  };

  enum PartStat {
    NeedsAction, Accepted, Declined, Tentative,
    Delegated, Completed, InProcess
  };

  enum Priority {
    High = 2, Normal = 3, Low = 1
  };

  enum AlarmAction {
    Display
  };

  // This set of functions sets all properties on the file you want to save
  void setSender(const QString &name, const QString &email);
  void setMessageType(MessageType m);
  void setMethod( Method m );
  void clearAttendees();
  void addAttendee( const QString& cn, Role r, PartStat p, bool rsvp,
                    const QString& mailto );
  void setOrganizer( const QString& organizer ); // Is that the same as sender???
  void setDtStart( const QDateTime& dtStart );
  void setDtEnd( const QDateTime& dtEnd );
  void setLocation( const QString& location );
  void setUID( const QString& uid );
  void setDtStamp( const QDateTime& dtStamp );
  void setCategories( const QStringList& );
  void setDescription( const QString& );
  void setSummary( const QString& );
  void setPriority( Priority p );
  void setAlarm( const QString& description, AlarmAction action,
                 const QDateTime& wakeBefore );

private:
  class PrivateData;
  PrivateData *mData;
};


#endif // KTNEFWRITER_H
