/*
  Copyright (C) 2010 Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>
  Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>
  Copyright (c) 2010 Eduardo Madeira Fleury <efleury@gmail.com>

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

#include <QTime>
#include <QtCore/qmath.h>

static const qreal Q_PI   = qreal(3.14159265358979323846);   // pi
static const qreal Q_2PI  = qreal(6.28318530717958647693);   // 2*pi

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


ClockHelper::ClockHelper(QObject *parent)
  : QObject(parent), m_origin(0, 0), m_position(0, 0),
    m_angle(0), m_minutes(0), m_minutesAngle(0), m_hours(0),
    m_hoursAngle(0), selected(ClockHelper::None)
{
  // init the clock with a sane value
  QTime time = QTime::currentTime();
  setHour( time.hour() );
  setMinute( time.minute() );
}

ClockHelper::~ClockHelper()
{
}

/*
  Calculate angle and radius based on current origin and position.
*/
void ClockHelper::calculateAngle()
{
  const qreal projectionInX = m_position.x() - m_origin.x();
  const qreal projectionInY = m_position.y() - m_origin.y();
  const qreal oldAngle = m_angle;

  const qreal radius = sqrt(projectionInX * projectionInX + projectionInY * projectionInY);

  if (radius == 0) {
    // Corner case
    m_angle = 0;
  } else if (projectionInX > 0) {
    if (projectionInY >= 0) {
      // First quadrant
      m_angle = qAsin(projectionInY / radius);
    } else {
      // Fourth quadrant
      m_angle = Q_2PI + qAsin(projectionInY / radius);
    }
  } else {
    // Second and Third quadrants
    m_angle = Q_PI - qAsin(projectionInY / radius);
  }

  // +90 because we need to adjust the clock '0 degree'
  m_angle = normalize(fromRadians(m_angle) + 90);

  if (oldAngle != m_angle) {
    switch(selected) {
    case ClockHelper::Hour:
      setHour( m_angle/30 );
      break;
    case ClockHelper::Minute:
      setMinute( m_angle/6 );
      break;
    case ClockHelper::None:
    default:
      break;
    }
  }
}

qreal ClockHelper::originX() const
{
  return m_origin.x();
}

void ClockHelper::setOriginX(qreal x)
{
  if (x == m_origin.x())
    return;

  m_origin.setX(x);
}

qreal ClockHelper::originY() const
{
  return m_origin.y();
}

void ClockHelper::setOriginY(qreal y)
{
  if (y == m_origin.y())
    return;

  m_origin.setY(y);
}

void ClockHelper::setXY(qreal x, qreal y)
{
  if (selected == ClockHelper::None)
    return;

  const bool xEqual = (x == m_position.x());
  const bool yEqual = (y == m_position.y());

  if (xEqual && yEqual)
    return;

  m_position.setX(x);
  m_position.setY(y);
  calculateAngle();
}

int ClockHelper::minutes() const
{
  return m_minutes;
}

int ClockHelper::minutesAngle() const
{
  return m_minutesAngle;
}

void ClockHelper::setMinute(int min)
{
  if (min == m_minutes)
    return;

  m_minutes = min;
  emit minutesChanged(min);

  // match our angle
  min = normalize(min) * 6;
  m_minutesAngle = min;
  emit minutesAngleChanged(min);
}

int ClockHelper::hours() const
{
  return m_hours;
}

int ClockHelper::hoursAngle() const
{
  return m_hoursAngle;
}

void ClockHelper::setHour(int hour)
{
  if (hour == m_hours)
    return;

  m_hours = hour;
  emit hoursChanged();

  // math our angle
  hour = normalize(hour) * 30;
  m_hoursAngle = hour;
  emit hoursAngleChanged();
}

void ClockHelper::selectMinute()
{
  selected = ClockHelper::Minute;
}

void ClockHelper::selectHour()
{
  selected = ClockHelper::Hour;
}

void ClockHelper::unselectAll()
{
  selected = ClockHelper::None;
}
