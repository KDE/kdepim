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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSYNC_SYNCERPART_H
#define KSYNC_SYNCERPART_H

#include <actionpart.h>

#include <synceelist.h>
#include <syncer.h>

#include <klocale.h>

#include <qpixmap.h>
#include <qmap.h>

class KAboutData;

class QListView;
class QTextView;

namespace KSync {

class SyncUiKde;
class KonnectorView;

class SyncerPart : public ActionPart
{
   Q_OBJECT
  public:
    SyncerPart( QWidget *parent, const char *name,
              QObject *object=0, const char *name2 = 0, // make GenericFactory loading possible
              const QStringList & = QStringList() );
    virtual ~SyncerPart();

    static KAboutData *createAboutData();

    QString type() const;
    QString title() const;
    QString description() const;
    bool hasGui() const;
    QPixmap *pixmap();
    QString iconName() const;
    QWidget *widget();

    bool needsKonnectorRead() const { return true; }
    bool needsKonnectorWrite() const { return true; }

    void logMessage( const QString & );

    void executeAction();

  private:
    QPixmap m_pixmap;
    QWidget *m_widget;

    KonnectorView *mKonnectorView;
    QTextView *mLogView;

    Syncer mCalendarSyncer;
    Syncer mAddressBookSyncer;

    SyncUiKde *mSyncUi;
};

}

#endif
