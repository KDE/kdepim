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
class KConfigGroup;

class SendLaterInfo
{
public:
    explicit SendLaterInfo();
    explicit SendLaterInfo(const KConfigGroup &config);
    SendLaterInfo(const SendLaterInfo &info);
    ~SendLaterInfo();

    enum RecursiveUnit {
        None = 0,
        Days,
        Weeks,
        Months
    };

    void setItemId(Akonadi::Item::Id id);
    Akonadi::Item::Id itemId() const;

    void setRecursiveUnit(RecursiveUnit unit);
    RecursiveUnit recursiveUnit() const;

    void setRecursiveEachValue(int value);
    int recursiveEachValue() const;

    bool isRecursive() const;
    void setRecursive(bool b);

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
    int mRecursiveEachValue;
    RecursiveUnit mRecursiveUnit;
    bool mRecursive;
};

#endif // SENDLATERINFO_H
