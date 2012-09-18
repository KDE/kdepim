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

#ifndef ARCHIVEMAILDIALOG_H
#define ARCHIVEMAILDIALOG_H

#include "ui_archivemailwidget.h"
#include "archivemailinfo.h"
#include <QTreeWidgetItem>
#include <QTreeWidget>

class ArchiveMailItem : public QTreeWidgetItem
{
public:
  explicit ArchiveMailItem(QTreeWidget *parent = 0);
  ~ArchiveMailItem();

  void setInfo(ArchiveMailInfo *info);
  ArchiveMailInfo *info() const;
private:
  ArchiveMailInfo *mInfo;
};

class ArchiveMailWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ArchiveMailWidget( QWidget *parent = 0 );
  ~ArchiveMailWidget();
  void save();
  void saveTreeWidgetHeader(KConfigGroup& group);
  void restoreTreeWidgetHeader(const QByteArray &group);
private:
  void load();
  void createOrUpdateItem(ArchiveMailInfo *info, ArchiveMailItem* item = 0);

  bool verifyExistingArchive(ArchiveMailInfo *info) const;

private Q_SLOTS:
  void slotRemoveItem();
  void slotModifyItem();
  void slotAddItem();
  void updateButtons();
  void slotOpenFolder();
  void customContextMenuRequested(const QPoint&);
private:
  Ui::ArchiveMailWidget *mWidget;
};

class ArchiveMailDialog : public KDialog
{
  Q_OBJECT
public:
  explicit ArchiveMailDialog(QWidget *parent = 0);
  ~ArchiveMailDialog();
protected Q_SLOTS:
  void slotSave();

private:
  void writeConfig();
  void readConfig();
  ArchiveMailWidget *mWidget;
};


#endif /* ARCHIVEMAILWIDGET_H */

