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
#ifndef SMS_H
#define SMS_H

#include <libkmobiletools/kmobiletools_export.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>

/**
@author Marco Gulino
*/

class SMSPrivate;
class KMOBILETOOLS_EXPORT SMS : public QObject
{
Q_OBJECT
public:
    SMS(QObject *parent = 0);
    SMS(const QStringList & numbers, const QString & text, QObject *parent = 0);
    SMS(const QStringList & numbers, const QString & text, const QDateTime & datetime, QObject *parent = 0);

    ~SMS();

    enum MemorySlot
    { SIM=0x1, Phone=0x2, Unknown=0x10 };
    enum SMSType
    { Unread=0x1, Read=0x2, Unsent=0x4, Sent=0x8, All=0xA };
    static QString SMSTypeString(SMSType smstype);

    static int SMSIntType (const QString& type);

    bool isIncoming() const;
    void setText(const QString & text);
    virtual QString getText() const;
    virtual QStringList getMultiText() const;
    static QStringList getMultiText(const QString&);
    static int getMultiTextCount(const QString&);
    static int getMultiTextCount(int);
    virtual QString getFrom() const;
    virtual QStringList getTo() const;
    virtual QString getDate() const;
    virtual QDateTime getDateTime() const;
    virtual void setRawSlot(const QString &rawSlot);
    virtual QString rawSlot() const;
    void setNumbers(const QStringList & numbers);
    void setDateTime(const QDateTime & datetime);

    void setFolder( int newFolder );
    int folder() const;
    QList<int> *idList();
//     void setId( int newId ) { i_id = newId; }; // krazy:exclude=inline

    void setSlot( int newSlot );
    SMSType type() const;
    void setType( SMSType newType );
    int slot() const;
    QByteArray uid() const;
    bool operator ==(SMS* compSMS);
    bool unread() const;
    void setUnread(bool unread);
private:
    SMSPrivate * const d;

public Q_SLOTS:
    bool exportMD(const QString &dir);
    bool writeToSlot( const QString &slotDir);
    bool exportCSV(const QString &filename);
    bool writeToSlotCSV( const QString &filename);
Q_SIGNALS:
    void updated();
};

#endif
