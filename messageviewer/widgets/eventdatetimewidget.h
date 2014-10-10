/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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
#include <KDateTime>
#include "messageviewer_export.h"
class KDateComboBox;
class KTimeComboBox;

namespace MessageViewer {
class MESSAGEVIEWER_EXPORT EventDateTimeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EventDateTimeWidget(QWidget *parent = 0);
    ~EventDateTimeWidget();

    void setMinimumDateTime(const KDateTime &dateTime);
    void setDateTime(const KDateTime &dateTime);
    KDateTime dateTime() const;

    QDate date() const;
    QTime time() const;
    void setTime(const QTime &time);
    void setDate(const QDate &date);

Q_SIGNALS:
    void dateTimeChanged(const KDateTime &dateTime);

private Q_SLOTS:
    void slotDateTimeChanged();
private:
    KDateComboBox *mDateEdit;
    KTimeComboBox *mTimeEdit;
};
}

#endif // EVENTDATETIMEWIDGET_H
