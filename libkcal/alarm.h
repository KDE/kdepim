/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#include <qvaluelist.h>

#include "customproperties.h"
#include "duration.h"
#include "person.h"

namespace KCal {

class Incidence;

/**
  This class represents an alarm notification.
*/
class Alarm : public CustomProperties
{
  public:
    enum Type { Display, Procedure, Email, Audio };
    typedef QValueList<Alarm *> List;

    /** Constructs a new alarm with variables initialized to "sane" values. */
    Alarm(Incidence *parent);
    /** Destruct Alarm object. */
    ~Alarm();

    /** return the type of the alarm */
    Type type() const;

    /** set the event to have this file as the noise for the alarm. */
    void setAudioFile(const QString &audioAlarmFile);
    /** return the name of the audio file for the alarm */
    QString audioFile() const;

    /** set this program to run when an alarm is triggered */
    void setProgramFile(const QString &programAlarmFile);
    /** return the name of the program to run when an alarm is triggered */
    QString programFile() const;

    /** send mail to this address when an alarm goes off */
    void setMailAddress(const Person &mailAlarmAddress);
    /** send mail to these addresses when an alarm goes off */
    void setMailAddresses(const QValueList<Person> &mailAlarmAddresses);
    /** add this address to the list of addresses to send mail to when an alarm goes off */
    void addMailAddress(const Person &mailAlarmAddress);
    /** return the addresses to send mail to when an alarm goes off */
    QValueList<Person> mailAddresses() const;

    /** set the subject line of the mail */
    void setMailSubject(const QString &mailAlarmSubject);
    /** return the subject line of the mail  */
    QString mailSubject() const;

    /** attach this filename to the email */
    void setMailAttachment(const QString &mailAttachFile);
    /** attach these filenames to the email */
    void setMailAttachments(const QStringList &mailAttachFiles);
    /** add this filename to the list of files to attach to the email */
    void addMailAttachment(const QString &mailAttachFile);
    /** return the filenames to attach to the email */
    QStringList mailAttachments() const;

    /** set the text to display when an alarm goes off */
    void setText(const QString &alarmText);
    /** return the text string that displays when an alarm goes off */
    QString text() const;

    /** set the time to trigger an alarm */
    void setTime(const QDateTime &alarmTime);
    /** return the date/time when an alarm goes off */
    QDateTime time() const;
    /** Return true, if the alarm has an explicit date/time. */
    bool hasTime() const;

    /** Set offset of alarm in time relative to the start of the event. */
    void setOffset( const Duration & );
    /** Return offset of alarm in time relative to the start of the event. */
    Duration offset() const;

    /** set the interval between snoozes for the alarm */
    void setSnoozeTime(int alarmSnoozeTime);
    /** get how long the alarm snooze interval is */
    int snoozeTime() const;

    /** set how many times an alarm is to repeat itself (w/snoozes) */
    void setRepeatCount(int alarmRepeatCount);
    /** get how many times an alarm repeats */
    int repeatCount() const;

    /** toggles the value of alarm to be either on or off.
        set's the alarm time to be x minutes before dtStart time. */
    void toggleAlarm();

    /** set the alarm enabled status */
    void setEnabled(bool enable);
    /** get the alarm enabled status */
    bool enabled() const;

    /** Set the alarm's parent incidence */
    void setParent( Incidence * );
    /** get the alarm's parent incidence */
    Incidence *parent() const  { return mParent; }

  private:
    QString mAudioAlarmFile;     // url/filename of sound to play
    QString mProgramAlarmFile;   // filename of program to run
    QStringList mMailAttachFiles;           // filenames to attach to email
    QValueList<Person> mMailAlarmAddresses; // who to mail for reminder
    QString mMailAlarmSubject;   // subject of email
    QString mAlarmText;          // text to display/mail for alarm

    int mAlarmSnoozeTime;        // number of minutes after alarm to
                                 // snooze before ringing again
    int mAlarmRepeatCount;       // number of times for alarm to repeat
                                 // after the initial time
    bool mAlarmEnabled;

    QDateTime mAlarmTime;        // time at which to display the alarm
    bool mHasTime;
    Duration mOffset;

    Incidence *mParent;
};

}

#endif
