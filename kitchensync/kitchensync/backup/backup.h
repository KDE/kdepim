/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNC_BACKUP_H
#define KSYNC_BACKUP_H

#include <actionpart.h>

#include <synceelist.h>

#include <libkcal/calendarlocal.h>

#include <klocale.h>

#include <qpixmap.h>
#include <qptrlist.h>
#include <qlistview.h>

class KAboutData;

class QTextView;

class CustomComboBox;

namespace KSync {

class KonnectorView;
class BackupView;

class Backup : public ActionPart
{
   Q_OBJECT
  public:
    Backup( QWidget *parent, const char *name,
            QObject *object = 0, const char *name2 = 0, // make GenericFactory loading possible
            const QStringList & = QStringList() );
    virtual ~Backup();

    static KAboutData *createAboutData();

    QString type() const;
    QString title() const;
    QString description() const;
    bool hasGui() const;
    QPixmap *pixmap();
    QString iconName() const;
    QWidget *widget();

    void logMessage( const QString & );

    void executeAction();

    bool needsKonnectorRead() const { return true; }

  protected:
    Konnector *currentKonnector();

    void updateRestoreList();

    QString topBackupDir() const;

    void tryFinishBackup();

    QString backupFile( Konnector *k, Syncee *s );

    void backupKonnector( Konnector *k );

  protected slots:
    void slotBackupDeleted( const QString & );
    
  private:
    QPixmap mPixmap;
    QWidget *mWidget;

    KonnectorView *mKonnectorList;
    BackupView *mBackupView;
    QTextView *mLogView;

    QPtrList<Konnector> mOpenedKonnectors;
    uint mKonnectorCount;

    QString mBackupDir;

    bool mActive;
};

}

#endif
