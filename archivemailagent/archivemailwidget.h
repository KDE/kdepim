/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ARCHIVEMAILWIDGET_H
#define ARCHIVEMAILWIDGET_H

#include "ui_archivemailwidget.h"
#include <QListWidgetItem>
#include <QListWidget>

class ArchiveMailItem : public QListWidgetItem
{
public:
  explicit ArchiveMailItem( const QString &text, QListWidget *parent = 0 );
  ~ArchiveMailItem();

 private:
};

class ArchiveMailWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ArchiveMailWidget( QWidget *parent = 0 );
  ~ArchiveMailWidget();
private:
  void load();
  void save();

private Q_SLOTS:
  void slotRemoveItem();
  void slotModifyItem();
  void slotAddItem();
private:
  Ui::ArchiveMailWidget *mWidget;
};


#endif /* ARCHIVEMAILWIDGET_H */

