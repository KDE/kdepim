/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/
#ifndef GROUPCONFIGCOMMON_H
#define GROUPCONFIGCOMMON_H

#include <qwidget.h>
#include <qmap.h>

#include <libqopensync/group.h>

class KLineEdit;
class SyncProcess;
class QCheckBox;

class ObjectTypeSelector : public QWidget
{
  public:
    ObjectTypeSelector( QWidget *parent );

    void load( const QSync::Group &group );
    void save( QSync::Group group );

  private:
    QMap<QString,QCheckBox *> mObjectTypeChecks;
};

class GroupConfigCommon : public QWidget
{
  public:
    GroupConfigCommon( QWidget *parent );

    void setSyncProcess( SyncProcess *syncProcess );
    void save();

  private:
    KLineEdit *mGroupName;
    ObjectTypeSelector *mObjectTypeSelector;

    SyncProcess *mSyncProcess;
};

#endif
