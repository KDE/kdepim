/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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


#ifndef PIMSETTINGEXPORTERWINDOW_H
#define PIMSETTINGEXPORTERWINDOW_H
#include <kxmlguiwindow.h>

class LogWidget;
class AbstractImportExportJob;

class PimSettingExporterWindow: public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit PimSettingExporterWindow(QWidget *parent=0);
    ~PimSettingExporterWindow();

private Q_SLOTS:
    void slotBackupData();
    void slotRestoreData();
    void slotAddInfo(const QString& info);
    void slotAddError(const QString& info);
    void slotSaveLog();

private:
    void executeJob();
    bool canZip() const;
    void setupActions(bool canZipFile);
    LogWidget *mLogWidget;
    AbstractImportExportJob *mImportExportData;
};


#endif /* PIMSETTINGEXPORTERWINDOW_H */

