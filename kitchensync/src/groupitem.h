/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
*/

#ifndef GROUPITEM_H
#define GROUPITEM_H

#include "syncprocess.h"

#include <libqopensync/callbackhandler.h>

#include <kwidgetlist.h>

namespace QSync {
class Engine;
}

class MemberItem;
class KURLLabel;
class QLabel;
class QProgressBar;
class QVBox;

class GroupItem : public KWidgetListItem
{
  Q_OBJECT

  public:
    GroupItem( KWidgetList*, SyncProcess *syncProcess );
    ~GroupItem();

    SyncProcess *syncProcess() const { return mSyncProcess; }

    void update();

    void clear();

  signals:
    void synchronizeGroup( SyncProcess *syncProcess );
    void abortSynchronizeGroup( SyncProcess *syncProcess );
    void configureGroup( SyncProcess *syncProcess );

  protected slots:
    void conflict( QSync::SyncMapping );
    void change( const QSync::SyncChangeUpdate& );
    void mapping( const QSync::SyncMappingUpdate& );
    void engine( const QSync::SyncEngineUpdate& );
    void member( const QSync::SyncMemberUpdate& );

    void synchronize();
    void configure();

    void engineChanged( QSync::Engine *engine );

  private:
    SyncProcess *mSyncProcess;
    QSync::CallbackHandler *mCallbackHandler;
    QValueList<MemberItem*> mMemberItems;

    QLabel *mIcon;
    QLabel *mGroupName;
    QLabel *mStatus;
    QLabel *mTime;
    KURLLabel *mSyncAction;
    KURLLabel *mConfigureAction;
    QVBox *mBox;
    QProgressBar *mProgressBar;

    int mProcessedItems;
    int mMaxProcessedItems;
    bool mSynchronizing;
};

class MemberItem : public QWidget
{
  public:
    MemberItem( QWidget *parent, SyncProcess *syncProcess,
                const QSync::Member &member );

    SyncProcess* syncProcess() const { return mSyncProcess; }
    QSync::Member member() const { return mMember; }

    void setStatusMessage( const QString &msg );

  private:
    SyncProcess *mSyncProcess;
    QSync::Member mMember;

    QLabel *mIcon;
    QLabel *mMemberName;
    QLabel *mDescription;
    QLabel *mStatus;
};

#endif
