/*
    This file is part of kdepim.

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

#ifndef ACTIONPAGE_H
#define ACTIONPAGE_H

#include "servertype.h"

#include <qwidget.h>

class QLabel;
class QListBox;
class QPushButton;

class ActionPage : public QWidget
{
  Q_OBJECT

  public:
    ActionPage( QWidget *parent, const char *name = 0 );
    ~ActionPage();

  public slots:
    void setServerType( const QString& );

  private slots:
    void selectionChanged();

    void addConnection();
    void editConnection();
    void deleteConnection();
    void activateConnection();

  private:
    void reloadConnections();

    QLabel *mTitleLabel;
    QListBox *mConnectionBox;
    QPushButton *mAddButton;
    QPushButton *mEditButton;
    QPushButton *mDeleteButton;
    QPushButton *mActivateButton;

    ServerType *mServerType;
    ServerType::ConnectionInfoList mConnectionInfoList;
};

#endif
