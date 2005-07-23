/*
    This file is part of KitchenSync.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSYNC_BACKUPVIEW_H
#define KSYNC_BACKUPVIEW_H

#include <klocale.h>

#include <qptrlist.h>
#include <qlistview.h>

namespace KSync {

class Konnector;
class Syncee;

class BackupView : public QWidget
{
   Q_OBJECT
  public:
    BackupView( QWidget *parent, const char *name = 0 );

    QString selectedBackup();

    void updateBackupList();

    void setBackupDir( const QString &dateStr );
    void createBackupDir();

    QString backupFile( Konnector *k, Syncee *s );

  signals:
    void backupDeleted( const QString & );

  protected:

    QString topBackupDir() const;

  protected slots:
    void deleteBackup();

  private:
    QListView *mBackupList;

    QString mBackupDir;
};

}

#endif
