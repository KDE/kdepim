/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EVENTDATETIMEWIDGET_H
#define EVENTDATETIMEWIDGET_H

#include <QWidget>
#include <QDateTime>
#include "messageviewer_export.h"

namespace MessageViewer
{
class EventDateTimeWidgetPrivate;
class MESSAGEVIEWER_EXPORT EventDateTimeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EventDateTimeWidget(QWidget *parent = Q_NULLPTR);
    ~EventDateTimeWidget();

    void setMinimumDateTime(const QDateTime &dateTime);
    void setDateTime(const QDateTime &dateTime);
    QDateTime dateTime() const;

    QDate date() const;
    QTime time() const;
    void setTime(const QTime &time);
    void setDate(const QDate &date);

Q_SIGNALS:
    void dateTimeChanged(const QDateTime &dateTime);

private Q_SLOTS:
    void slotDateTimeChanged();
private:
    EventDateTimeWidgetPrivate *const d;
};
}

#endif // EVENTDATETIMEWIDGET_H
