/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#ifndef KSYNC_SYNCUIKDEPLUGIN_H
#define KSYNC_SYNCUIKDEPLUGIN_H

#include <qwidget.h>

namespace KSync {

class SyncEntry;

/**
  The SyncUiKDEPlugin is similiar to the
  plugin(s) found in RenameDlg of KDE
  in KIO::RenameDlg the mimetype get's determined
  and an appropriate Plugin gets loaded.
  SyncUIKDE differs because it loads for each
  source and destination a plugin.
*/
class SyncUiKDEPlugin : public QWidget
{
  public:
    SyncUiKDEPlugin( QWidget* parent,  const char *name, const QStringList& list )
        : QWidget( parent, name ) {}
    virtual ~SyncUiKDEPlugin() {}
    virtual void setSyncEntry( SyncEntry* entry ) = 0
};

}

#endif
