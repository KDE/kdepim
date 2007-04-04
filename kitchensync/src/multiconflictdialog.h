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

#ifndef MULTICONFLICTDIALOG_H
#define MULTICONFLICTDIALOG_H

#include "conflictdialog.h"

class KWidgetList;

class MultiConflictDialog : public ConflictDialog
{
  Q_OBJECT

  public:
    MultiConflictDialog( QSync::SyncMapping &mapping, QWidget *parent );
    ~MultiConflictDialog();

  private slots:
    void useSelectedChange();
    void duplicateChange();
    void ignoreChange();

  private:
    void initGUI();

    KWidgetList *mWidgetList;
};

#endif
