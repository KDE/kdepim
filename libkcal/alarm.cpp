/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
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

#include <kdebug.h>

#include "incidence.h"
#include "todo.h"

#include "alarm.h"

using namespace KCal;

Alarm::Alarm(Incidence *parent)
{
  mParent = parent;

  mAlarmSnoozeTime = 5;
  mAlarmRepeatCount = 0;
  mAlarmEnabled = false;
  mProgramAlarmFile = ""; // to make the comparison not fail
  mAudioAlarmFile = ""; // to make the comparison not fail

  mHasTime = false;
}

Alarm::~Alarm()
{
}

bool KCal::operator==( const Alarm& a1, const Alarm& a2 )
{
    return ( a1.audioFile() == a2.audioFile() &&
             a1.programFile() == a2.programFile() &&
             a1.mailAttachments() == a2.mailAttachments() &&
             a1.mailAddresses() == a2.mailAddresses() &&
             a1.mailSubject() == a2.mailSubject() &&
             a1.text() == a2.text() &&
             a1.snoozeTime() == a2.snoozeTime() &&
             a1.repeatCount() == a2.repeatCount() &&
             a1.enabled() == a2.enabled() &&
             a1.hasTime() == a2.hasTime() &&
             a1.offset() == a2.offset() );
}


Alarm::Type Alarm::type() const
{
  if (!mProgramAlarmFile.isEmpty())
    return Procedure;
  if (!mAudioAlarmFile.isEmpty())
    return Audio;
  if (mMailAlarmAddresses.count() > 0)
    return Email;
  return Display;
}

void Alarm::setAudioFile(const QString &audioAlarmFile)
{
  mAudioAlarmFile = audioAlarmFile;
  mParent->updated();
}

QString Alarm::audioFile() const
{
  return mAudioAlarmFile;
}

void Alarm::setProgramFile(const QString &programAlarmFile)
{
  mProgramAlarmFile = programAlarmFile;
  mParent->updated();
}

QString Alarm::programFile() const
{
  return mProgramAlarmFile;
}

void Alarm::setMailAddress(const Person &mailAlarmAddress)
{
  mMailAlarmAddresses.clear();
  mMailAlarmAddresses += mailAlarmAddress;
  mParent->updated();
}

void Alarm::setMailAddresses(const QValueList<Person> &mailAlarmAddresses)
{
  mMailAlarmAddresses = mailAlarmAddresses;
  mParent->updated();
}

void Alarm::addMailAddress(const Person &mailAlarmAddress)
{
  mMailAlarmAddresses += mailAlarmAddress;
  mParent->updated();
}

QValueList<Person> Alarm::mailAddresses() const
{
  return mMailAlarmAddresses;
}

void Alarm::setMailSubject(const QString &mailAlarmSubject)
{
  mMailAlarmSubject = mailAlarmSubject;
  mParent->updated();
}

QString Alarm::mailSubject() const
{
  return mMailAlarmSubject;
}

void Alarm::setMailAttachment(const QString &mailAttachFile)
{
  mMailAttachFiles.clear();
  mMailAttachFiles += mailAttachFile;
  mParent->updated();
}

void Alarm::setMailAttachments(const QStringList &mailAttachFiles)
{
  mMailAttachFiles = mailAttachFiles;
  mParent->updated();
}

void Alarm::addMailAttachment(const QString &mailAttachFile)
{
  mMailAttachFiles += mailAttachFile;
  mParent->updated();
}

QStringList Alarm::mailAttachments() const
{
  return mMailAttachFiles;
}

void Alarm::setText(const QString &alarmText)
{
  mAlarmText = alarmText;
  mParent->updated();
}

QString Alarm::text() const
{
  return mAlarmText;
}

void Alarm::setTime(const QDateTime &alarmTime)
{
  mAlarmTime = alarmTime;
  mHasTime = true;

  mParent->updated();
}

QDateTime Alarm::time() const
{
  if ( hasTime() ) return mAlarmTime;
  else
  {
    if (mParent->type()=="Todo") {
      Todo *t = static_cast<Todo*>(mParent);
      return mOffset.end( t->dtDue() );
    } else {
      return mOffset.end( mParent->dtStart() );
    }
  }
}

bool Alarm::hasTime() const
{
  return mHasTime;
}

void Alarm::setSnoozeTime(int alarmSnoozeTime)
{
  mAlarmSnoozeTime = alarmSnoozeTime;
  mParent->updated();
}

int Alarm::snoozeTime() const
{
  return mAlarmSnoozeTime;
}

void Alarm::setRepeatCount(int alarmRepeatCount)
{
  kdDebug(5800) << "Alarm::setRepeatCount(): " << alarmRepeatCount << endl;

  mAlarmRepeatCount = alarmRepeatCount;
  mParent->updated();
}

int Alarm::repeatCount() const
{
  kdDebug(5800) << "Alarm::repeatCount(): " << mAlarmRepeatCount << endl;
  return mAlarmRepeatCount;
}

void Alarm::toggleAlarm()
{
  if (mAlarmEnabled) {
    mAlarmEnabled = false;
  } else {
    mAlarmEnabled = true;
  }

  mParent->updated();
}

void Alarm::setEnabled(bool enable)
{
  mAlarmEnabled = enable;
  mParent->updated();
}

bool Alarm::enabled() const
{
  return mAlarmEnabled;
}

void Alarm::setOffset( const Duration &offset )
{
  mOffset = offset;
  mHasTime = false;

  mParent->updated();
}

Duration Alarm::offset() const
{
  return mOffset;
}

void Alarm::setParent( Incidence *parent )
{
  mParent = parent;
}
