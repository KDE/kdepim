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
#ifndef KSYNC_PROFILEWIZARD_H
#define KSYNC_PROFILEWIZARD_H

#include "actionpartservice.h"
#include "profile.h"

#include <kdialogbase.h>

class KLineEdit;
class KListView;

namespace KSync {

class ProfileCheckItem;

// FIXME: This is not a wizard, but the ProfileConfigureDialog
class ProfileWizard : public KDialogBase
{
    Q_OBJECT
  public:
    ProfileWizard( const ActionPartService::List &availableParts );
    ProfileWizard( const Profile &,
                   const ActionPartService::List &availableParts );
    ~ProfileWizard();

    Profile profile();

    ActionPartService::List selectedActionParts();

  protected slots:
    void slotOk();

    void addPart();
    void removePart();
    void raisePart();
    void lowerPart();

    ProfileCheckItem *selectedItem();

  private:
    void initUI();
    void initProfile();

    Profile mProfile;
    ActionPartService::List mAvailableParts;
    KLineEdit *mNameEdit;
    KListView *mPartListView;
};

}

#endif
