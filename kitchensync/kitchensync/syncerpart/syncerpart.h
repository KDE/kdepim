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
#ifndef KSYNC_SYNCERPART
#define KSYNC_SYNCERPART

#include <manipulatorpart.h>

#include <synceelist.h>
#include <syncer.h>

#include <libkcal/calendarlocal.h>

#include <klocale.h>

#include <qpixmap.h>
#include <qmap.h>

class KAboutData;

class QListView;
class QTextView;

namespace KSync {

class SyncerPart : public ManipulatorPart
{
   Q_OBJECT
  public:
    SyncerPart( QWidget *parent, const char *name,
              QObject *object=0, const char *name2 = 0, // make GenericFactory loading possible
              const QStringList & = QStringList() );
    virtual ~SyncerPart();

    static KAboutData *createAboutData();

    QString type() const;
    QString name() const;
    QString description() const;
    bool partIsVisible() const;
    QPixmap *pixmap();
    QString iconName() const;
    QWidget *widget();

    void logMessage( const QString & );

    void actionSync();

  public slots:
    void slotSynceesRead( Konnector *, const SynceeList & );

    void slotSynceeReadError( Konnector * );

    void slotSynceesWritten( Konnector * );

    void slotSynceeWriteError( Konnector * );

  protected:
    void updateKonnectorList();

    void trySync();
    void tryFinishSync();
    
    void disconnectDevice( Konnector *k );

  protected slots:
    void slotProgress( Konnector *, const Progress & );
    void slotError( Konnector *, const Error & );

  private:
    QPixmap m_pixmap;
    QWidget *m_widget;

    QListView *mKonnectorList;
    QTextView *mLogView;

    SynceeList mSynceeList;

    QMap<QString,Konnector *> mKonnectorMap;

    QPtrList<Konnector> mOpenedKonnectors;
    QPtrList<Konnector> mProcessedKonnectors;
    uint mKonnectorCount;

    Syncer mCalendarSyncer;
    Syncer mAddressBookSyncer;
};

}

#endif
