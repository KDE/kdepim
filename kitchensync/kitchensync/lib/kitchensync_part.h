/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KITCHENSYNC_PART_H
#define KITCHENSYNC_PART_H

#include <kitchensync.h>

#include <kurl.h>
#include <kparts/factory.h>
#include <kparts/part.h>

class KInstance;
class KAboutData;
class KProcess;

namespace KSync {

class ActionManager;

}

class KitchenSyncPart: public KParts::ReadOnlyPart
{
    Q_OBJECT
  public:
    KitchenSyncPart( QWidget *parentWidget, const char *widgetName,
                     QObject *parent, const char *name, const QStringList & );
    virtual ~KitchenSyncPart();

    static KAboutData *createAboutData();

  protected:
    virtual bool openFile();

  private:
    KSync::ActionManager *mActionManager;
};

#endif
