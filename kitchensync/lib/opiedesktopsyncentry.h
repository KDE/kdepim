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

#ifndef opiedesktopsyncentry_h
#define opiedesktopsyncentry_h

#include "ksyncentry.h"

/** OpieDesktopSyncEntry
 *  OpieDesktopSyncEntry is Qtopia and Opie specific.
 *  All files in the Document tab are represented with one
 *  OpieDesktopSyncEntry
 *
 */

class OpieDesktopSyncEntry : public KSyncEntry{
 public:
    //c 'tor
    OpieDesktopSyncEntry();
    OpieDesktopSyncEntry(const QString &category, const QString &file, const QString &name, const QString &type, const QString &size  );
    ~OpieDesktopSyncEntry();


    virtual QString type() { return QString::fromLatin1("OpieDesktopEntry" ); };
    /**
     * @return the Name of this OpieDesktopSyncEntry
     */
    virtual QString name(); // Zweck entfremded
    QString file() const;
    QString type() const;
    QString size() const;
    virtual void setName(const QString & ) { };
    virtual QString id();
    virtual void setId(const QString & );
    virtual QString oldId();
    virtual void setOldId(const QString & );

    virtual QString timestamp();
    virtual void setTimestamp(const QString & );
    virtual bool equals(KSyncEntry * );
    virtual KSyncEntry* clone();

 private:
    QString m_name;
    QString m_file;
    QString m_id;
    QString m_category;
    QString m_type;
    QString m_size;
};
#endif



