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

#ifndef KONNECTORPAIRVIEW_H
#define KONNECTORPAIRVIEW_H

#include <klistview.h>

class KonnectorPairManager;

using namespace KSync;

class KonnectorPairItem : public QObject, public QListViewItem
{
  Q_OBJECT

  public:
    KonnectorPairItem( KonnectorPair *pair, KListView *parent );

    QString text( int column ) const;
    QString uid() const;

  private slots:
    void synceesRead( Konnector* );
    void synceeReadError( Konnector* );
    void synceesWritten( Konnector* );
    void synceeWriteError( Konnector* );

  private:
    KonnectorPair *mPair;
    QString mStatusMsg;
};

class KonnectorPairView : public KListView
{
  Q_OBJECT

  public:
    KonnectorPairView( KonnectorPairManager* manager, QWidget *parent );
    ~KonnectorPairView();

    QString selectedPair() const;

    void refresh();

  private slots:
    void refreshView();

  private:
    KonnectorPairManager *mManager;
};

#endif
