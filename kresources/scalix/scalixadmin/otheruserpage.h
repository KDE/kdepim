/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef OTHERUSERPAGE_H
#define OTHERUSERPAGE_H

#include <qwidget.h>

#include "otherusermanager.h"

class QPushButton;
class OtherUserView;

class OtherUserPage : public QWidget
{
  Q_OBJECT

  public:
    OtherUserPage( QWidget *parent = 0 );
    ~OtherUserPage();

  private slots:
    void loadAllUsers();
    void addUser();
    void removeUser();

    void userAdded( KIO::Job* );
    void userRemoved( KIO::Job* );
    void allUsers( KIO::Job* );

    void selectionChanged();

  private:
    void updateKmail();

    QPushButton *mAddButton;
    QPushButton *mDeleteButton;

    OtherUserManager mManager;
    OtherUserView *mView;
};

#endif
