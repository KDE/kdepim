/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef SENDLATERINFO_H
#define SENDLATERINFO_H

#include <Akonadi/Item>
#include <QDateTime>
#include "sendlater_export.h"

class KConfigGroup;

namespace SendLater {

class SENDLATER_EXPORT SendLaterInfo
{
public:
    explicit SendLaterInfo();
    explicit SendLaterInfo(const KConfigGroup &config);
    SendLaterInfo(const SendLater::SendLaterInfo &info);
    ~SendLaterInfo();

    enum RecurrenceUnit {
        Days = 0,
        Weeks,
        Months,
        Years
    };

    void setItemId(Akonadi::Item::Id id);
    Akonadi::Item::Id itemId() const;

    void setRecurrenceUnit(RecurrenceUnit unit);
    RecurrenceUnit recurrenceUnit() const;

    void setRecurrenceEachValue(int value);
    int recurrenceEachValue() const;

    bool isRecurrence() const;
    void setRecurrence(bool b);

    void setDateTime(const QDateTime &time);
    QDateTime dateTime() const;

    void readConfig(const KConfigGroup &config);
    void writeConfig(KConfigGroup &config );

    void setLastDateTimeSend( const QDateTime &date );
    QDateTime lastDateTimeSend() const;

    void setSubject( const QString &subject );
    QString subject() const;

private:
    QString mSubject;
    QDateTime mDateTime;
    QDateTime mLastDateTimeSend;
    Akonadi::Item::Id mId;
    int mRecurrenceEachValue;
    RecurrenceUnit mRecurrenceUnit;
    bool mRecurrence;
};
}

#endif // SENDLATERINFO_H
