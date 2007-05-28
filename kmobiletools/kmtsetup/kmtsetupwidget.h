/***************************************************************************
   Copyright (C) 2007
   by Davide Bettio <davide.bettio@kdemail.net>
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/


#ifndef _KMTSETUPWIDGET_H_
#define _KMTSETUPWIDGET_H_

#include <qstringlist.h>
#include <q3valuelist.h>
#include <kuser.h>
#include <kdialogbase.h>

#include "kmtsetupwidgetbase.h"

class KMTSetupWidget : public KDialogBase
{
    Q_OBJECT

public:
    explicit KMTSetupWidget(QWidget* parent = 0, const char* name = 0 );
    ~KMTSetupWidget();

public slots:
    void slotOk();
    void slotCancel();
    void slotDefault();
    void btnAddDevice_clicked();
    void remUserClicked();
    void userListClicked( Q3ListViewItem * item );


private:
    void addUserToGroup(const QString &user, const QString &group);
    void loadGroupsLists();
    void loadUsersLists();
    QStringList groupslist;
    QStringList userslist;
    KMTSetupWidgetBase *p_widget;
};

#endif

