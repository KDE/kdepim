/*
    This file is part of libkcal.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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
*/

#ifndef KCAL_ALARM_H
#define KCAL_ALARM_H

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "customproperties.h"
#include "duration.h"
#include "person.h"
#include "listbase.h"

#include <kdepimmacros.h>

namespace KCal {

class Incidence;

/**
  This class represents an alarm notification.
*/
class KDE_EXPORT Alarm : public CustomProperties
{
  public:
    enum Type { Invalid, Display, Procedure, Email, Audio };

    typedef ListBase<Alarm> List;

    /**
      Construct a new alarm with variables initialized to "sane" values.
    */
    explicit Alarm( Incidence *parent );
    /**
      Destruct Alarm object.
    */
    ~Alarm();

    /**
      Compare this alarm with another one.
    */
    bool operator==( const Alarm & ) const;
    bool operator!=( const Alarm &a ) const { return !operator==( a ); }

    /**
      Set the type of the alarm.
      If the specified type is different from the current type of the alarm,
      the alarm's type-specific properties are initialised to null.
      
      @param type type of alarm.
    */
    void setType( Type type );
    /**
      Return the type of the alarm.
    */
    Type type() const;

    /**
      Set the alarm to be a display alarm.
      
      @param text text to display when the alarm is triggered.
    */
    void setDisplayAlarm( const QString &text = QString::null );
    /**
      Set the text to be displayed when the alarm is triggered.
      Ignored if the alarm is not a display alarm.
    */
    void setText( const QString &text );
    /**
      Return the text string that displays when the alarm is triggered.
    */
    QString text() const;

    /**
      Set the alarm to be an audio alarm.
      
      @param audioFile optional file to play when the alarm is triggered.
    */
    void setAudioAlarm( const QString &audioFile = QString::null );
    /**
      Set the file to play when the audio alarm is triggered.
      Ignored if the alarm is not an audio alarm.
    */
    void setAudioFile( const QString &audioFile );
    /**
      Return the name of the audio file for the alarm.
      
      @return The audio file for the alarm, or QString::null if not an audio alarm.
    */
    QString audioFile() const;

    /**
      Set the alarm to be a procedure alarm.
      
      @param programFile program to execute when the alarm is triggered.
      @param arguments arguments to supply to programFile.
    */
    void setProcedureAlarm( const QString &programFile,
                            const QString &arguments = QString::null );
    /**
      Set the program file to execute when the alarm is triggered.
      Ignored if the alarm is not a procedure alarm.
    */
    void setProgramFile( const QString &programFile );
    /**
      Return the name of the program file to execute when the alarm is triggered.
      
      @return the program file name, or QString::null if not a procedure alarm.
    */
    QString programFile() const;
    /**
      Set the arguments to the program to execute when the alarm is triggered.
      Ignored if the alarm is not a procedure alarm.
    */
    void setProgramArguments( const QString &arguments );
    /**
      Return the arguments to the program to run when the alarm is triggered.
      
      @return the program arguments, or QString::null if not a procedure alarm.
    */
    QString programArguments() const;

    /**
      Set the alarm to be an email alarm.
      
      @param subject subject line of email.
      @param text body of email.
      @param addressees email addresses of recipient(s).
      @param attachments optional names of files to attach to the email.
    */
    void setEmailAlarm( const QString &subject, const QString &text,
                        const QValueList<Person> &addressees,
                        const QStringList &attachments = QStringList() );

    /**
      Send mail to this address when the alarm is triggered.
      Ignored if the alarm is not an email alarm.
    */
    void setMailAddress( const Person &mailAlarmAddress );
    /**
      Send mail to these addresses when the alarm is triggered.
      Ignored if the alarm is not an email alarm.
    */
    void setMailAddresses( const QValueList<Person> &mailAlarmAddresses );
    /**
      Add this address to the list of addresses to send mail to when the alarm is triggered.
      Ignored if the alarm is not an email alarm.
    */
    void addMailAddress( const Person &mailAlarmAddress );
    /**
      Return the addresses to send mail to when an alarm goes off.
    */
    QValueList<Person> mailAddresses() const;

    /**
      Set the subject line of the mail.
      Ignored if the alarm is not an email alarm.
    */
    void setMailSubject( const QString &mailAlarmSubject );
    /**
      Return the subject line of the mail.
    */
    QString mailSubject() const;

    /**
      Attach this filename to the email.
      Ignored if the alarm is not an email alarm.
    */
    void setMailAttachment( const QString &mailAttachFile );
    /**
      Attach these filenames to the email.
      Ignored if the alarm is not an email alarm.
    */
    void setMailAttachments( const QStringList &mailAttachFiles );
    /**
      Add this filename to the list of files to attach to the email.
      Ignored if the alarm is not an email alarm.
    */
    void addMailAttachment( const QString &mailAttachFile );
    /**
      Return the filenames to attach to the email.
    */
    QStringList mailAttachments() const;

    /**
      Set the email body text.
      Ignored if the alarm is not an email alarm.
    */
    void setMailText( const QString &text );
    /**
      Return the email body text.
      
      @return the body text, or QString::null if not an email alarm.
    */
    QString mailText() const;

    /**
      Set the time to trigger an alarm.
    */
    void setTime( const QDateTime &alarmTime );
    /**
      Return the date/time when an alarm goes off.
    */
    QDateTime time() const;
    /**
      Return the date/time when the last repetition of the alarm goes off.
	  If the alarm does not repeat, this is equivalent to calling time().
    */
    QDateTime endTime() const;
    /**
      Return true, if the alarm has an explicit date/time.
    */
    bool hasTime() const;

    /**
      Set offset of alarm in time relative to the start of the event.
    */
    void setStartOffset( const Duration & );
    /**
      Return offset of alarm in time relative to the start of the event.
      If the alarm's time is not defined in terms of an offset relative
      to the start of the event, returns zero.
    */
    Duration startOffset() const;
    /**
      Return whether the alarm is defined in terms of an offset relative
      to the start of the event.
    */
    bool hasStartOffset() const;

    /**
      Set offset of alarm in time relative to the end of the event.
    */
    void setEndOffset( const Duration & );
    /**
      Return offset of alarm in time relative to the end of the event.
      If the alarm's time is not defined in terms of an offset relative
      to the end of the event, returns zero.
    */
    Duration endOffset() const;
    /**
      Return whether the alarm is defined in terms of an offset relative
      to the end of the event.
    */
    bool hasEndOffset() const;

    /**
      Set the interval between snoozes for the alarm.
      
      @param alarmSnoozeTime the time in minutes between snoozes.
    */
    void setSnoozeTime( int alarmSnoozeTime );

    /**
      Get how long the alarm snooze interval is.
      
      @return the number of minutes between snoozes.
    */
    int snoozeTime() const;

    /**
      Set how many times an alarm is to repeat itself after its initial
      occurrence (w/snoozes).
    */
    void setRepeatCount( int alarmRepeatCount );
    /**
      Get how many times an alarm repeats, after its initial occurrence.
    */
    int repeatCount() const;
    /**
      Get the time of the alarm's initial occurrence or its next repetition,
      after a given time.
      @param preTime the date and time after which to find the next repetition.
      @return the date and time of the next repetition, or an invalid date/time if
      the specified time is at or after the alarm's last repetition.
    */
    QDateTime nextRepetition(const QDateTime& preTime) const;
    /**
      Get the time of the alarm's latest repetition, or its initial occurrence if
      none, before a given time.
      @param afterTime the date and time before which to find the latest repetition.
      @return the date and time of the latest repetition, or an invalid date/time if
      the specified time is at or before the alarm's initial occurrence.
    */
    QDateTime previousRepetition(const QDateTime& afterTime) const;
    /**
      Get how long between the alarm's initial occurrence and its final repetition.
      @return the number of seconds between the initial occurrence and final repetition.
    */
    int duration() const;

    /**
      Toggles the value of alarm to be either on or off.
      Set's the alarm time to be x minutes before dtStart time.
    */
    void toggleAlarm();

    /**
      Set the alarm enabled status.
    */
    void setEnabled(bool enable);
    /**
      Get the alarm enabled status.
    */
    bool enabled() const;

    /**
      Set the alarm's parent incidence.
    */
    void setParent( Incidence * );
    /**
      Get the alarm's parent incidence.
    */
    Incidence *parent() const  { return mParent; }

  private:
    Incidence *mParent;          // the incidence which this alarm belongs to
    Type mType;                  // type of alarm
    QString mDescription;        // text to display/email body/procedure arguments
    QString mFile;               // procedure program to run/optional audio file to play
    QStringList mMailAttachFiles;      // filenames to attach to email
    QValueList<Person> mMailAddresses; // who to mail for reminder
    QString mMailSubject;        // subject of email

    int mAlarmSnoozeTime;        // number of minutes after alarm to
                                 // snooze before ringing again
    int mAlarmRepeatCount;       // number of times for alarm to repeat
                                 // after the initial time

    QDateTime mAlarmTime;        // time at which to trigger the alarm
    Duration mOffset;            // time relative to incidence DTSTART to trigger the alarm
    bool mEndOffset;             // if true, mOffset relates to DTEND, not DTSTART
    bool mHasTime;               // use mAlarmTime, not mOffset
    bool mAlarmEnabled;

    class Private;
    Private *d;
};

}

#endif
