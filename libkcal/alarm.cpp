/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>

#include "incidence.h"
#include "todo.h"

#include "alarm.h"

using namespace KCal;

Alarm::Alarm(Incidence *parent)
 : mParent(parent),
   mType(Invalid),
   mDescription(""),    // to make operator==() not fail
   mFile(""),           // to make operator==() not fail
   mMailSubject(""),    // to make operator==() not fail
   mAlarmSnoozeTime(5),
   mAlarmRepeatCount(0),
   mEndOffset(false),
   mHasTime(false),
   mAlarmEnabled(false)
{
}

Alarm::~Alarm()
{
}

Alarm *Alarm::clone()
{
  return new Alarm( *this );
}

Alarm &Alarm::operator=( const Alarm &a )
{
  mParent = a.mParent;
  mType = a.mType;
  mDescription = a.mDescription;
  mFile = a.mFile;
  mMailAttachFiles = a.mMailAttachFiles;
  mMailAddresses = a.mMailAddresses;
  mMailSubject = a.mMailSubject;
  mAlarmSnoozeTime = a.mAlarmSnoozeTime;
  mAlarmRepeatCount = a.mAlarmRepeatCount;
  mAlarmTime = a.mAlarmTime;
  mOffset = a.mOffset;
  mEndOffset = a.mEndOffset;
  mHasTime = a.mHasTime;
  mAlarmEnabled = a.mAlarmEnabled;
  return *this;
}

bool Alarm::operator==( const Alarm& rhs ) const
{
  if ( mType != rhs.mType ||
       mAlarmSnoozeTime != rhs.mAlarmSnoozeTime ||
       mAlarmRepeatCount != rhs.mAlarmRepeatCount ||
       mAlarmEnabled != rhs.mAlarmEnabled ||
       mHasTime != rhs.mHasTime)
    return false;

  if (mHasTime) {
    if (mAlarmTime != rhs.mAlarmTime)
      return false;
  } else {
    if (mOffset != rhs.mOffset ||
        mEndOffset != rhs.mEndOffset)
      return false;
  }

  switch (mType) {
    case Display:
      return mDescription == rhs.mDescription;

    case Email:
      return mDescription == rhs.mDescription &&
             mMailAttachFiles == rhs.mMailAttachFiles &&
             mMailAddresses == rhs.mMailAddresses &&
             mMailSubject == rhs.mMailSubject;

    case Procedure:
      return mFile == rhs.mFile &&
             mDescription == rhs.mDescription;

    case Audio:
      return mFile == rhs.mFile;

    case Invalid:
      break;
  }
  return false;
}

void Alarm::setType(Alarm::Type type)
{
  if (type == mType)
    return;

  switch (type) {
    case Display:
      mDescription = "";
      break;
    case Procedure:
      mFile = mDescription = "";
      break;
    case Audio:
      mFile = "";
      break;
    case Email:
      mMailSubject = mDescription = "";
      mMailAddresses.clear();
      mMailAttachFiles.clear();
      break;
    case Invalid:
      break;
    default:
      return;
  }
  mType = type;
  if ( mParent ) mParent->updated();
}

Alarm::Type Alarm::type() const
{
  return mType;
}

void Alarm::setAudioAlarm(const TQString &audioFile)
{
  mType = Audio;
  mFile = audioFile;
  if ( mParent ) mParent->updated();
}

void Alarm::setAudioFile(const TQString &audioFile)
{
  if (mType == Audio) {
    mFile = audioFile;
    if ( mParent ) mParent->updated();
  }
}

TQString Alarm::audioFile() const
{
  return (mType == Audio) ? mFile : TQString::null;
}

void Alarm::setProcedureAlarm(const TQString &programFile, const TQString &arguments)
{
  mType = Procedure;
  mFile = programFile;
  mDescription = arguments;
  if ( mParent ) mParent->updated();
}

void Alarm::setProgramFile(const TQString &programFile)
{
  if (mType == Procedure) {
    mFile = programFile;
    if ( mParent ) mParent->updated();
  }
}

TQString Alarm::programFile() const
{
  return (mType == Procedure) ? mFile : TQString::null;
}

void Alarm::setProgramArguments(const TQString &arguments)
{
  if (mType == Procedure) {
    mDescription = arguments;
    if ( mParent ) mParent->updated();
  }
}

TQString Alarm::programArguments() const
{
  return (mType == Procedure) ? mDescription : TQString::null;
}

void Alarm::setEmailAlarm(const TQString &subject, const TQString &text,
                          const TQValueList<Person> &addressees, const TQStringList &attachments)
{
  mType = Email;
  mMailSubject = subject;
  mDescription = text;
  mMailAddresses = addressees;
  mMailAttachFiles = attachments;
  if ( mParent ) mParent->updated();
}

void Alarm::setMailAddress(const Person &mailAddress)
{
  if (mType == Email) {
    mMailAddresses.clear();
    mMailAddresses += mailAddress;
    if ( mParent ) mParent->updated();
  }
}

void Alarm::setMailAddresses(const TQValueList<Person> &mailAddresses)
{
  if (mType == Email) {
    mMailAddresses = mailAddresses;
    if ( mParent ) mParent->updated();
  }
}

void Alarm::addMailAddress(const Person &mailAddress)
{
  if (mType == Email) {
    mMailAddresses += mailAddress;
    if ( mParent ) mParent->updated();
  }
}

TQValueList<Person> Alarm::mailAddresses() const
{
  return (mType == Email) ? mMailAddresses : TQValueList<Person>();
}

void Alarm::setMailSubject(const TQString &mailAlarmSubject)
{
  if (mType == Email) {
    mMailSubject = mailAlarmSubject;
    if ( mParent ) mParent->updated();
  }
}

TQString Alarm::mailSubject() const
{
  return (mType == Email) ? mMailSubject : TQString::null;
}

void Alarm::setMailAttachment(const TQString &mailAttachFile)
{
  if (mType == Email) {
    mMailAttachFiles.clear();
    mMailAttachFiles += mailAttachFile;
    if ( mParent ) mParent->updated();
  }
}

void Alarm::setMailAttachments(const TQStringList &mailAttachFiles)
{
  if (mType == Email) {
    mMailAttachFiles = mailAttachFiles;
    if ( mParent ) mParent->updated();
  }
}

void Alarm::addMailAttachment(const TQString &mailAttachFile)
{
  if (mType == Email) {
    mMailAttachFiles += mailAttachFile;
    if ( mParent ) mParent->updated();
  }
}

TQStringList Alarm::mailAttachments() const
{
  return (mType == Email) ? mMailAttachFiles : TQStringList();
}

void Alarm::setMailText(const TQString &text)
{
  if (mType == Email) {
    mDescription = text;
    if ( mParent ) mParent->updated();
  }
}

TQString Alarm::mailText() const
{
  return (mType == Email) ? mDescription : TQString::null;
}

void Alarm::setDisplayAlarm(const TQString &text)
{
  mType = Display;
  if ( !text.isNull() )
    mDescription = text;
  if ( mParent ) mParent->updated();
}

void Alarm::setText(const TQString &text)
{
  if (mType == Display) {
    mDescription = text;
    if ( mParent ) mParent->updated();
  }
}

TQString Alarm::text() const
{
  return (mType == Display) ? mDescription : TQString::null;
}

void Alarm::setTime(const TQDateTime &alarmTime)
{
  mAlarmTime = alarmTime;
  mHasTime = true;

  if ( mParent ) mParent->updated();
}

TQDateTime Alarm::time() const
{
  if ( hasTime() ) {
    return mAlarmTime;
  } else if ( mParent ) {
    if ( mEndOffset ) {
      if ( mParent->type() == "Todo" ) {
        Todo *t = static_cast<Todo*>( mParent );
        return mOffset.end( t->dtDue() );
      } else {
        return mOffset.end( mParent->dtEnd() );
      }
    } else {
      return mOffset.end( mParent->dtStart() );
    }
  } else {
    return TQDateTime();
  }
}

bool Alarm::hasTime() const
{
  return mHasTime;
}

void Alarm::setSnoozeTime(const Duration &alarmSnoozeTime)
{
  if (alarmSnoozeTime.value() > 0) {
    mAlarmSnoozeTime = alarmSnoozeTime;
    if ( mParent ) mParent->updated();
  }
}

Duration Alarm::snoozeTime() const
{
  return mAlarmSnoozeTime;
}

void Alarm::setRepeatCount(int alarmRepeatCount)
{
  mAlarmRepeatCount = alarmRepeatCount;
  if ( mParent ) mParent->updated();
}

int Alarm::repeatCount() const
{
  return mAlarmRepeatCount;
}

Duration Alarm::duration() const
{
  return Duration( mAlarmSnoozeTime.value() * mAlarmRepeatCount,
                   mAlarmSnoozeTime.type() );
}

TQDateTime Alarm::nextRepetition(const TQDateTime& preTime) const
{
  // This method is coded to avoid 32-bit integer overflow using
  // TQDateTime::secsTo(), which occurs with time spans > 68 years.
  TQDateTime at = time();
  if (at > preTime)
    return at;
  if (!mAlarmRepeatCount)
    return TQDateTime();   // there isn't an occurrence after the specified time
  int snoozeSecs = mAlarmSnoozeTime * 60;
  TQDateTime lastRepetition = at.addSecs(mAlarmRepeatCount * snoozeSecs);
  if (lastRepetition <= preTime)
    return TQDateTime();    // all repetitions have finished before the specified time
  int repetition = (at.secsTo(preTime) + snoozeSecs) / snoozeSecs;
  return at.addSecs(repetition * snoozeSecs);
}

TQDateTime Alarm::previousRepetition(const TQDateTime& afterTime) const
{
  // This method is coded to avoid 32-bit integer overflow using
  // TQDateTime::secsTo(), which occurs with time spans > 68 years.
  TQDateTime at = time();
  if (at >= afterTime)
    return TQDateTime();    // alarm's first/only time is at/after the specified time
  if (!mAlarmRepeatCount)
    return at;
  int snoozeSecs = mAlarmSnoozeTime * 60;
  TQDateTime lastRepetition = at.addSecs(mAlarmRepeatCount * snoozeSecs);
  if (lastRepetition < afterTime)
    return lastRepetition;   // all repetitions have finished before the specified time
  int repetition = (at.secsTo(afterTime) - 1) / snoozeSecs;
  return at.addSecs(repetition * snoozeSecs);
}

TQDateTime Alarm::endTime() const
{
  if (mAlarmRepeatCount)
    return time().addSecs(mAlarmRepeatCount * mAlarmSnoozeTime * 60);
  else
    return time();
}

void Alarm::toggleAlarm()
{
  mAlarmEnabled = !mAlarmEnabled;
  if ( mParent ) mParent->updated();
}

void Alarm::setEnabled(bool enable)
{
  mAlarmEnabled = enable;
  if ( mParent ) mParent->updated();
}

bool Alarm::enabled() const
{
  return mAlarmEnabled;
}

void Alarm::setStartOffset( const Duration &offset )
{
  mOffset = offset;
  mEndOffset = false;
  mHasTime = false;
  if ( mParent ) mParent->updated();
}

Duration Alarm::startOffset() const
{
  return (mHasTime || mEndOffset) ? Duration( 0 ) : mOffset;
}

bool Alarm::hasStartOffset() const
{
  return !mHasTime && !mEndOffset;
}

bool Alarm::hasEndOffset() const
{
  return !mHasTime && mEndOffset;
}

void Alarm::setEndOffset( const Duration &offset )
{
  mOffset = offset;
  mEndOffset = true;
  mHasTime = false;
  if ( mParent ) mParent->updated();
}

Duration Alarm::endOffset() const
{
  return (mHasTime || !mEndOffset) ? Duration( 0 ) : mOffset;
}

void Alarm::setParent( Incidence *parent )
{
  mParent = parent;
}

void Alarm::customPropertyUpdated()
{
  if ( mParent ) mParent->updated();
}
