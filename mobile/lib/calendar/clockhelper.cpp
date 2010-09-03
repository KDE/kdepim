/*
  Copyright (C) 2010 Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/


#include "clockhelper.h"
#include "clockhelper_p.h"
#include <QDebug>
#include <QtCore/qmath.h>

static const qreal Q_PI   = qreal(3.14159265358979323846);   // pi
static const qreal Q_2PI  = qreal(6.28318530717958647693);   // 2*pi

ClockHelperPrivate::ClockHelperPrivate(ClockHelper *qq)
  : q_ptr(qq), origin(0, 0), position(0, 0), angle(0),
    seconds(0), minutes(0), hours(0), secondsHandSelected(false),
    minutesHandSelected(false), hoursHandSelected(false)

{
}

ClockHelperPrivate::~ClockHelperPrivate()
{
}

static inline qreal normalize(qreal angle)
{
  return angle - 360 * qFloor(angle / 360);   // qreal modulus 360
}

static inline qreal toRadians(qreal angle)
{
  return angle * Q_2PI / 360;
}

static inline qreal fromRadians(qreal angle)
{
  return angle * 360 / Q_2PI;
}

/*!
  \internal

  Calculate angle and radius based on current origin and position.
*/
void ClockHelperPrivate::calculateAngle()
{
  Q_Q(ClockHelper);

  const qreal projectionInX = position.x() - origin.x();
  const qreal projectionInY = position.y() - origin.y();
  const qreal oldAngle = angle;

  const qreal radius = sqrt(projectionInX * projectionInX + projectionInY * projectionInY);

  if (radius == 0) {
    // Corner case
    angle = 0;
  } else if (projectionInX > 0) {
    if (projectionInY >= 0) {
      // First quadrant
      angle = qAsin(projectionInY / radius);
    } else {
      // Fourth quadrant
      angle = Q_2PI + qAsin(projectionInY / radius);
    }
  } else {
    // Second and Third quadrants
    angle = Q_PI - qAsin(projectionInY / radius);
  }

  // +90 because we need to adjust the clock '0 degree'
  angle = normalize(fromRadians(angle) + 90);

  if (oldAngle != angle) {
    if (secondsHandSelected)
      q->setSeconds(angle);
    else if (minutesHandSelected)
      q->setMinutes(angle);
    else if (hoursHandSelected)
      q->setHours(angle);
  }
}

ClockHelper::ClockHelper(QObject *parent)
  : QObject(parent), d_ptr(new ClockHelperPrivate(this))
{
}

ClockHelper::~ClockHelper()
{
}

qreal ClockHelper::originX() const
{
  Q_D(const ClockHelper);
  return d->origin.x();
}

void ClockHelper::setOriginX(qreal x)
{
  Q_D(ClockHelper);
  if (x == d->origin.x())
    return;

  d->origin.setX(x);
}

qreal ClockHelper::originY() const
{
  Q_D(const ClockHelper);
  return d->origin.y();
}

void ClockHelper::setOriginY(qreal y)
{
  Q_D(ClockHelper);
  if (y == d->origin.y())
    return;

  d->origin.setY(y);
}

void ClockHelper::setXY(qreal x, qreal y)
{
  Q_D(ClockHelper);

  const bool xEqual = (x == d->position.x());
  const bool yEqual = (y == d->position.y());

  if (xEqual && yEqual)
    return;

  d->position.setX(x);
  d->position.setY(y);
  d->calculateAngle();
}

int ClockHelper::seconds() const
{
  Q_D(const ClockHelper);
  return d->seconds;
}

void ClockHelper::setSeconds(int sec)
{
  Q_D(ClockHelper);

  sec = normalize(sec) * 6;
  if (sec == d->seconds)
    return;

  d->seconds = sec;
  emit secondsChanged(sec);
}

int ClockHelper::minutes() const
{
  Q_D(const ClockHelper);
  return d->minutes;
}

void ClockHelper::setMinutes(int min)
{
  Q_D(ClockHelper);

  min = normalize(min) * 6;
  if (min == d->minutes)
    return;

  d->minutes = min;
  emit minutesChanged(min);
}

int ClockHelper::hours() const
{
  Q_D(const ClockHelper);
  return d->hours;
}

void ClockHelper::setHours(int hour)
{
  Q_D(ClockHelper);

  hour = normalize(hour) * 30;
  if (hour == d->hours)
    return;

  d->hours = hour;
  emit hoursChanged(hour);
}

void ClockHelper::setSecondsHandSelected(bool selected)
{
  Q_D(ClockHelper);
  d->secondsHandSelected = selected;
  d->minutesHandSelected = !selected;
  d->hoursHandSelected = !selected;
}

bool ClockHelper::secondsHandSelected() const
{
  Q_D(const ClockHelper);
  return d->secondsHandSelected;
}

void ClockHelper::setMinutesHandSelected(bool selected)
{
  Q_D(ClockHelper);
  d->secondsHandSelected = !selected;
  d->minutesHandSelected = selected;
  d->hoursHandSelected = !selected;
}

bool ClockHelper::minutesHandSelected() const
{
  Q_D(const ClockHelper);
  return d->minutesHandSelected;
}

void ClockHelper::setHoursHandSelected(bool selected)
{
  Q_D(ClockHelper);
  d->secondsHandSelected = !selected;
  d->minutesHandSelected = !selected;
  d->hoursHandSelected = selected;
}

bool ClockHelper::hoursHandSelected() const
{
  Q_D(const ClockHelper);
  return d->hoursHandSelected;
}
