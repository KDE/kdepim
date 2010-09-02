/*
    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>

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

#ifndef CALENDARHELPER_H
#define CALENDARHELPER_H

#include <QObject>
#include <QDateTime>


class CalendarHelper : public QObject
{
  Q_OBJECT;
  Q_PROPERTY(int day READ day WRITE setDay NOTIFY dayChanged)
  Q_PROPERTY(int month READ month WRITE setMonth NOTIFY monthChanged)
  Q_PROPERTY(QString monthName READ monthName NOTIFY monthChanged)
  Q_PROPERTY(int year READ year WRITE setYear NOTIFY yearChanged)
  Q_PROPERTY(QDateTime date READ date WRITE setDate NOTIFY dateChanged)

  public:
    CalendarHelper( QObject *parent = 0 );
    ~CalendarHelper();

    QDateTime date() const;
    void setDate( const QDateTime datetime );

    int day() const;
    void setDay( const int day );

    int month() const;
    QString monthName() const;
    void setMonth( const int month );

    int year() const;
    void setYear( const int year );

    Q_INVOKABLE QString dayForPosition( const int pos ) const;
    Q_INVOKABLE int weekForPosition( const int pos ) const;
    Q_INVOKABLE bool isCurrentDay( const QString &text ) const;

  signals:
    void dateChanged();
    void dayChanged();
    void monthChanged();
    void yearChanged();

  private:
    QDateTime m_original;
    int m_day;
    int m_month;
    int m_year;
    int m_offset;
    int m_weekOffset;
    int m_daysInMonth;
};

#endif
