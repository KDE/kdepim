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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef XXPORTMANAGER_H
#define XXPORTMANAGER_H

#include <tqdict.h>
#include <tqobject.h>

#include <kurl.h>

#include <kdepimmacros.h>

#include "xxport.h"

namespace KAB {
class Core;
}

class KDE_EXPORT XXPortManager : public QObject
{
  Q_OBJECT

  public:
    XXPortManager( KAB::Core *core, TQObject *parent, const char *name = 0 );
    ~XXPortManager();

    void restoreSettings();
    void saveSettings();

    static KURL importURL;
    static TQString importData;

  public slots:
    void importVCard( const KURL &url );
    void importVCardFromData( const TQString &vCard );

  signals:
    void modified();

  protected slots:
    void slotImport( const TQString&, const TQString& );
    void slotExport( const TQString&, const TQString& );

  private:
    void loadPlugins();

    TQDict<KAB::XXPort> mXXPortObjects;

    KAB::Core *mCore;
};

#endif
