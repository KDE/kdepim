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

#ifndef CLOCKHELPER_H
#define CLOCKHELPER_H

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtDeclarative/qdeclarative.h>

class ClockHelperPrivate;
class ClockHelper : public QObject
{
  Q_OBJECT

  Q_PROPERTY(qreal originX READ originX WRITE setOriginX)
  Q_PROPERTY(qreal originY READ originY WRITE setOriginY)
  Q_PROPERTY(bool minutesHandSelected READ minutesHandSelected WRITE setMinutesHandSelected)
  Q_PROPERTY(bool hoursHandSelected READ hoursHandSelected WRITE setHoursHandSelected)
  Q_PROPERTY(int minutes READ minutes WRITE setMinute NOTIFY minutesChanged)
  Q_PROPERTY(int minutesAngle READ minutesAngle NOTIFY minutesAngleChanged)
  Q_PROPERTY(int hours READ hours WRITE setHour NOTIFY hoursChanged)
  Q_PROPERTY(int hoursAngle READ hoursAngle NOTIFY hoursAngleChanged)

public:
  ClockHelper(QObject *parent = 0);
  virtual ~ClockHelper();

  qreal originX() const;
  void setOriginX(qreal x);

  qreal originY() const;
  void setOriginY(qreal x);

  Q_INVOKABLE void setXY(qreal x, qreal y);

  int minutes() const;
  int minutesAngle() const;
  void setMinute(int min);

  int hours() const;
  int hoursAngle() const;
  void setHour(int hour);

  void setMinutesHandSelected(bool selected);
  bool minutesHandSelected() const;

  void setHoursHandSelected(bool selected);
  bool hoursHandSelected() const;

Q_SIGNALS:
  void minutesChanged();
  void minutesAngleChanged();
  void hoursChanged();
  void hoursAngleChanged();

protected:
  QScopedPointer<ClockHelperPrivate> d_ptr;

private:
  Q_DISABLE_COPY(ClockHelper)
  Q_DECLARE_PRIVATE(ClockHelper)
};

QML_DECLARE_TYPE(ClockHelper)

#endif // CLOCKHELPER_H
