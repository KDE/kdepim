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

// $Id$

#include "incidence.h"

#include "alarm.h"

using namespace KCal;

Alarm::Alarm(Incidence *parent)
{
  mParent = parent;

  mAlarmReadOnly = false;

  mAudioAlarmFile = "";
  mProgramAlarmFile = "";
  mMailAlarmAddress = "";
  mAlarmText = "";

  mAlarmSnoozeTime = 5;
  mAlarmRepeatCount = 0;
  mAlarmEnabled = false;
}

Alarm::~Alarm()
{
}

void Alarm::setAudioFile(const QString &audioAlarmFile)
{
  if (mAlarmReadOnly) return;
  mAudioAlarmFile = audioAlarmFile;
  mParent->emitEventUpdated(mParent);
}

const QString &Alarm::audioFile() const
{
  return mAudioAlarmFile;
}

void Alarm::setProgramFile(const QString &programAlarmFile)
{
  if (mAlarmReadOnly) return;
  mProgramAlarmFile = programAlarmFile;
  mParent->emitEventUpdated(mParent);
}

const QString &Alarm::programFile() const
{
  return mProgramAlarmFile;
}

void Alarm::setMailAddress(const QString &mailAlarmAddress)
{
  if (mAlarmReadOnly) return;
  mMailAlarmAddress = mailAlarmAddress;
  mParent->emitEventUpdated(mParent);
}

const QString &Alarm::mailAddress() const
{
  return mMailAlarmAddress;
}

void Alarm::setMailSubject(const QString &mailAlarmSubject)
{
  if (mAlarmReadOnly) return;
  mMailAlarmSubject = mailAlarmSubject;
  mParent->emitEventUpdated(mParent);
}

const QString &Alarm::mailSubject() const
{
  return mMailAlarmSubject;
}

void Alarm::setText(const QString &alarmText)
{
  if (mAlarmReadOnly) return;
  mAlarmText = alarmText;
  mParent->emitEventUpdated(mParent);
}

const QString &Alarm::text() const
{
  return mAlarmText;
}

void Alarm::setTime(const QDateTime &alarmTime)
{
  if (mAlarmReadOnly) return;
  mAlarmTime = alarmTime;
  mParent->emitEventUpdated(mParent);
}

const QDateTime &Alarm::time() const
{
  return mAlarmTime;
}

void Alarm::setSnoozeTime(int alarmSnoozeTime)
{
  if (mAlarmReadOnly) return;
  mAlarmSnoozeTime = alarmSnoozeTime;
  mParent->emitEventUpdated(mParent);
}

int Alarm::snoozeTime() const
{
  return mAlarmSnoozeTime;
}

void Alarm::setRepeatCount(int alarmRepeatCount)
{
  if (mAlarmReadOnly) return;
  mAlarmRepeatCount = alarmRepeatCount;
  mParent->emitEventUpdated(mParent);
}

int Alarm::repeatCount() const
{
  return mAlarmRepeatCount;
}

void Alarm::toggleAlarm()
{
  if (mAlarmReadOnly) return;
  if (mAlarmEnabled) {
    mAlarmEnabled = false;
  } else {
    mAlarmEnabled = true;
//    QString alarmStr(QString::number(KOPrefs::instance()->mAlarmTime));
// TODO: Fix default alarm time
    QString alarmStr("10");
    int pos = alarmStr.find(' ');
    if (pos >= 0)
      alarmStr.truncate(pos);
    mAlarmTime = mAlarmStart.addSecs(-60 * alarmStr.toUInt());
  }
  mParent->emitEventUpdated(mParent);
}

void Alarm::setEnabled(bool enable)
{
  if (mAlarmReadOnly) return;
  mAlarmEnabled = enable;
  mParent->emitEventUpdated(mParent);
}

bool Alarm::enabled() const
{
  return mAlarmEnabled;
}

