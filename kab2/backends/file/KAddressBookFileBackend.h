/*
    KAddressBook version 2
    
    Copyright (C) 1999 The KDE PIM Team <kde-pim@kde.org>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef KADDRESSBOOK_FILE_BACKEND_H
#define KADDRESSBOOK_FILE_BACKEND_H

// KDE includes
#include <klibloader.h>

#include "KAddressBookBackend.h"

class KAddressBookFileBackendFactory : public KLibFactory
{
  Q_OBJECT

  public:

    KAddressBookFileBackendFactory(QObject * parent = 0, const char * name = 0);
    ~KAddressBookFileBackendFactory();

  protected:

    QObject * createObject
      (
       QObject * parent = 0,
       const char * name = 0,
       const char * /* classname */ = "QObject",
       const QStringList & args = QStringList()
      );
};

class KAddressBookFileBackend : public KAddressBookBackend
{
  Q_OBJECT

  public:

    KAddressBookFileBackend
      (
       QString name,
       QString path,
       QObject * parent = 0,
       const char * name = 0
      );

    virtual ~KAddressBookFileBackend();

    virtual bool        contains(QString) const;
    virtual Entry       entry(QString)    const;
    virtual QStringList entryList()       const;

    virtual QString     insert(Entry);
    virtual bool        remove(QString);
    virtual bool        replace(Entry);

  private:

    bool      _checkDirs();
    void      _initIndex();

    Entry     _readEntry(const QString & filename) const;
    bool      _writeEntry(const Entry &);
    bool      _removeEntry(const QString & id);

    QString   _generateUniqueID();

    QStringList index_;
    QString uniquePartOne_;
};

#endif
