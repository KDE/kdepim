/*
    ktnefwriter.cpp

    Copyright (C) 2002 Bo Thorsen  <bo@sonofthor.dk>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef KTNEFWRITER_H
#define KTNEFWRITER_H

class TQString;
class TQVariant;
class TQIODevice;
class TQDataStream;
class TQDateTime;
class TQStringList;

class KTNEFWriter {
public:
  KTNEFWriter();
  ~KTNEFWriter();

  void addProperty( int tag, int type, const TQVariant& value );

  bool writeFile( TQIODevice &file );
  bool writeFile( TQDataStream &stream );

  bool writeProperty( TQDataStream &stream, int &bytes, int type);

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
  void setSender(const TQString &name, const TQString &email);
  void setMessageType(MessageType m);
  void setMethod( Method m );
  void clearAttendees();
  void addAttendee( const TQString& cn, Role r, PartStat p, bool rsvp,
                    const TQString& mailto );
  void setOrganizer( const TQString& organizer ); // Is that the same as sender???
  void setDtStart( const TQDateTime& dtStart );
  void setDtEnd( const TQDateTime& dtEnd );
  void setLocation( const TQString& location );
  void setUID( const TQString& uid );
  void setDtStamp( const TQDateTime& dtStamp );
  void setCategories( const TQStringList& );
  void setDescription( const TQString& );
  void setSummary( const TQString& );
  void setPriority( Priority p );
  void setAlarm( const TQString& description, AlarmAction action,
                 const TQDateTime& wakeBefore );

private:
  class PrivateData;
  PrivateData *mData;
};


#endif // KTNEFWRITER_H
