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


#ifndef BACKUPMAILWINDOW_H
#define BACKUPMAILWINDOW_H
#include <kxmlguiwindow.h>

class BackupMailWidget;
class ExportMailJob;
class ImportMailJob;

class BackupMailWindow: public KXmlGuiWindow
{
  Q_OBJECT
public:
  explicit BackupMailWindow(QWidget *parent=0);
  ~BackupMailWindow();
private Q_SLOTS:
  void slotBackupData();
  void slotRestoreData();
  void slotAddInfo(const QString& info);
  void slotAddError(const QString& info);
  void slotBackupDataFinished();
  void slotRestoreDataFinished();
private:
  bool canZip() const;
  void setupActions(bool canZipFile);
  BackupMailWidget *mBackupMailWidget;
  ExportMailJob *mBackupData;
  ImportMailJob *mRestoreData;
};


#endif /* BackupMailWindow_H */

