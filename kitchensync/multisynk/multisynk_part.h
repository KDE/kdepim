/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef MULTISYNK_PART_H
#define MULTISYNK_PART_H

#include <kparts/event.h>
#include <kparts/factory.h>
#include <libkdepim/part.h>

class KAboutData;

class MultiSynkPart: public KPIM::Part
{
  Q_OBJECT

  public:
    MultiSynkPart( QWidget *parentWidget, const char *widgetName,
                   QObject *parent, const char *name, const QStringList& );
    virtual ~MultiSynkPart();

    static KAboutData *createAboutData();

    virtual void exit();
    virtual bool openURL( const KURL &url );

  protected:
    virtual bool openFile();
    virtual void guiActivateEvent( KParts::GUIActivateEvent* );
};

#endif
