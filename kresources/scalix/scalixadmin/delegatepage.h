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

#ifndef DELEGATEPAGE_H
#define DELEGATEPAGE_H

#include <qwidget.h>

#include "delegatemanager.h"

class QPushButton;
class DelegateView;

class DelegatePage : public QWidget
{
  Q_OBJECT

  public:
    DelegatePage( QWidget *parent = 0 );
    ~DelegatePage();

  private slots:
    void loadAllDelegates();
    void addDelegate();
    void editDelegate();
    void removeDelegate();

    void delegateAdded( KIO::Job* );
    void delegateRemoved( KIO::Job* );
    void allDelegates( KIO::Job* );

    void selectionChanged();

  private:
    QPushButton *mAddButton;
    QPushButton *mEditButton;
    QPushButton *mRemoveButton;

    DelegateManager mManager;
    DelegateView *mView;
};

#endif
