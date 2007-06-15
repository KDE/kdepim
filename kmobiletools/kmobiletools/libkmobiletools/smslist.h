/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef SMSLIST_H
#define SMSLIST_H

#include <libkmobiletools/kmobiletools_export.h>

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include "sms.h"

/**
@author Marco Gulino
*/

class SMSListPrivate;

class KMOBILETOOLS_EXPORT SMSList : public QObject, public QList<SMS*>
{
    Q_OBJECT
public:
    SMSList(const QString &enginename=QString());

    ~SMSList();
    void append(SMSList *sublist, bool sync=false);
    void append(SMS *sms);
    int find(const QByteArray &uid) const;
    void sync (SMSList *compList);
    void dump() const;
    void calcSMSNumber() const;
    int count(int smsType, int memSlot) const;
    void resetCount() const;
    void setEngineName(const QString &enginename);
    QString engineName() const ;
protected:
//     int compareItems( Q3PtrCollection::Item item1, Q3PtrCollection::Item item2);
private:
    SMSListPrivate *const d;

Q_SIGNALS:
    void removed(const QByteArray&);
    void added(const QByteArray&);
    void modified(const QByteArray&);
    void updated();

public Q_SLOTS:
    void saveToMailBox(const QString &engineName);
    void saveToMailBox() const;
    int saveToCSV(const QString &filename);
    int saveToCSV();
};

#endif
