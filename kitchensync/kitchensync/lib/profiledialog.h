/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNC_PROFILEDIALOG_H
#define KSYNC_PROFILEDIALOG_H

#include "profile.h"
#include "actionpartservice.h"

#include <kdialogbase.h>

class KListView;
class QPushButton;

namespace KSync {

class ProfileDialog : public KDialogBase
{
    Q_OBJECT
  public:
    ProfileDialog( const Profile::List &,
                   const ActionPartService::List &availableParts );
    ~ProfileDialog();
    Profile::List profiles()const;

  private slots:
    void slotRemove();
    void slotAdd();
    void slotEdit();

    void slotSelectionChanged();

  private:
    void initListView( const Profile::List & );

    ActionPartService::List mAvailableParts;
    KListView *mProfileList;
    QPushButton *mEditButton;
    QPushButton *mRemoveButton;
};

}

#endif
