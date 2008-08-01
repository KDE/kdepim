/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef XXPORTMANAGER_H
#define XXPORTMANAGER_H

#include "xxport.h"

#include <QtCore/QHash>
#include <QtCore/QObject>

#include <kurl.h>

namespace KAB {
class Core;
}

class KADDRESSBOOK_EXPORT XXPortManager : public QObject
{
  Q_OBJECT

  public:
    XXPortManager( KAB::Core *core, QObject *parent, const char *name = 0 );
    ~XXPortManager();

    void restoreSettings();
    void saveSettings();

    static KUrl importURL;
    static QString importData;

  public Q_SLOTS:
    void importVCard( const KUrl &url );
    void importVCardFromData( const QString &vCard );

  Q_SIGNALS:
    void modified();

  protected Q_SLOTS:
    void slotImport( const QString&, const QString& );
    void slotExport( const QString&, const QString& );

  private:
    void loadPlugins();

    QHash< QString, KAB::XXPort * > mXXPortObjects;

    KAB::Core *mCore;
};

#endif
