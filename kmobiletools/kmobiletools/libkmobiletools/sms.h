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
#include <kmime/kmime_content.h>
#include <kmime/kmime_headers.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <kdatetime.h>
/**
@author Marco Gulino
*/

class SMSPrivate;
class SenderPrivate;
namespace KMobileTools {
typedef QHash<QString,QString> PhoneNumbers;

class KMOBILETOOLS_EXPORT Sender : public KMime::Headers::Generics::Structured
{
public:
    Sender();
    virtual ~Sender();
    virtual const char *type() const;
    virtual void addNumber(const QString &number, const QString &displayname=QString() );
    virtual void clear();
    virtual bool isEmpty() const;
    virtual bool parse(const char *&scursor, const char *const send, bool isCRLF=false);
    virtual QByteArray as7BitString(bool withHeaderType=true) const;
    PhoneNumbers phoneNumbers() const;
private:
    SenderPrivate *const d;
};
/*class KMOBILETOOLS_EXPORT Destination : public KMime::Headers::Generics::Structured
{
public:
    Destination();
    virtual ~Destination();
    virtual const char *type();
};
*/

class KMOBILETOOLS_EXPORT SMS : public KMime::Content
{
public:
    SMS();
    KDE_DEPRECATED SMS(const QStringList & numbers, const QString & text);
    KDE_DEPRECATED SMS(const QStringList & numbers, const QString & text, const KDateTime & datetime);

    virtual ~SMS();

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

    virtual KDE_DEPRECATED QString getFrom() const;
    virtual QStringList getTo() const;
    void KDE_DEPRECATED setNumbers(const QStringList & numbers);
    virtual QString getDate() const;
    virtual KDateTime getDateTime() const;
    virtual void setRawSlot(const QString &rawSlot);
    virtual QString rawSlot() const;
    void setDateTime(const KDateTime & datetime);

    void setFolder( int newFolder );
    int folder() const;
    QList<int> *idList();
//     void setId( int newId ) { i_id = newId; }; // krazy:exclude=inline

    void setSlot( int newSlot );
    SMSType type() const;
    void setType( SMSType newType );
    int slot() const;
    QString uid() const;
    bool operator ==(SMS* compSMS);
    bool unread() const;
    void setUnread(bool unread);
    bool writeToSlot( const QString &slotDir);
    bool exportMD(const QString &dir);
    bool exportCSV(const QString &filename);
    bool writeToSlotCSV( const QString &filename);

    // Headers
    KMime::Headers::Date *date() const;
   Sender *sender() const;
//    Destination *to()   const;

   void setSender(const QString& number, const QString &displayname=QString() );
//    void addTo  (const QString& number, const QString &displayname=QString() );

protected:
    virtual QByteArray assembleHeaders();

private:
    SMSPrivate *const d;
/// @TODO signals and slots removed, now find a way to do this in smslist.
/*public Q_SLOTS:

Q_SIGNALS:
    void updated();*/
};
}

#endif
