/* This file is part of the KDE libraries
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/



#ifndef kunknownsyncentry_h
#define kunknownsyncentry_h

#include <qdatetime.h>
#include <ksyncentry.h>

/** This is the KUnknownSyncEntry
 *  If a konnector is requested to load a file but
 *  is not able to handle it or convert it it will
 *  return a KUnknownSyncEntry
 */

class KUnknownSyncEntry : public KSyncEntry
{
public:
    KUnknownSyncEntry( );
    KUnknownSyncEntry(const QString &name, const QString &id, const QString &fileName  );
    ~KUnknownSyncEntry();
    virtual QString type() { return QString::fromLatin1("KUnknownSyncEntry"); };
    virtual QString name();
    virtual void setName(const QString &name );
    virtual QString id();
    virtual void setId( const QString & );
    virtual QString oldId();
    virtual void setOldId(const QString &);
    virtual QString timestamp();
    virtual void setTimestamp(const QString & );

    virtual bool equals(KUnknownSyncEntry * );

    /** This will return the file as a ByteArray
     * @return This returns the file requested as a QByteArray
     */
    QByteArray byteArray() const;
     /**
     * this sets the QByteArray
     */
    void setByteArray(const QByteArray &byteArray);

    /**
     *  set the filename where the file comes from
     */
    void setSrcFileName(const QString &fileName );
    /**
     * @return This returns the fileName where the QByteArray got
     * retrieved from
     */
    QString fileName() const;

    virtual bool equals(KSyncEntry *) { return false; }
    virtual KSyncEntry* clone();

private:
    class KUnknownSyncEntryPrivate;
    KUnknownSyncEntryPrivate *d;
};

#endif





