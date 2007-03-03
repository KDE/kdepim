/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef GROUPVIEW_H
#define GROUPVIEW_H

#include <kwidgetlist.h>

class AboutPage;
class SyncProcess;
class QVBoxLayout;

class GroupView : public QWidget
{
  Q_OBJECT

  public:
    GroupView( QWidget *parent );

    SyncProcess* selectedSyncProcess() const;

    void clear();

  public slots:
    void updateView();
    void updateSyncProcess( SyncProcess *process );

  signals:
    void addGroup();
    void synchronizeGroup( SyncProcess *syncProcess );
    void abortSynchronizeGroup( SyncProcess *syncProcess );
    void configureGroup( SyncProcess *syncProcess );

  private:
    AboutPage *mAboutPage;
    KWidgetList *mWidgetList;

    QVBoxLayout *mLayout;
};

#endif
