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
#include "utils.h"
class LogWidget;
class AbstractImportExportJob;
class KRecentFilesAction;
class KUrl;
class ArchiveStorage;
class KAction;

class PimSettingExporterWindow: public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit PimSettingExporterWindow(QWidget *parent=0);
    ~PimSettingExporterWindow();
    void handleCommandLine();

private Q_SLOTS:
    void slotJobFinished();
    void slotBackupData();
    void slotRestoreData();
    void slotAddInfo(const QString &info);
    void slotAddError(const QString &info);
    void slotAddTitle(const QString &info);
    void slotAddEndLine();
    void slotSaveLog();
    void slotShowStructureInfos();
    void slotRestoreFile(const KUrl &url);
    void slotShowArchiveInformations();

private:
    enum Action {
        Backup,
        Restore
    };

    void updateActions(bool inAction);
    void backupNextStep();
    void restoreNextStep();
    void backupFinished();
    void backupStart();
    void restoreFinished();
    void restoreStart();
    void backupData(const QString &filename, const QString &templateFile = QString());
    void loadData(const QString &filename, const QString &templateFile = QString());
    void executeJob();
    bool canZip() const;
    void setupActions(bool canZipFile);
    QHash<Utils::AppsType, Utils::importExportParameters> mStored;
    QHash<Utils::AppsType, Utils::importExportParameters>::const_iterator mStoreIterator;
    Action mAction;
    LogWidget *mLogWidget;
    AbstractImportExportJob *mImportExportData;
    KRecentFilesAction *mRecentFilesAction;
    ArchiveStorage *mArchiveStorage;
    KAction *mBackupAction;
    KAction *mRestoreAction;
    KAction *mSaveLogAction;
    KAction *mArchiveStructureInfo;
    KAction *mShowArchiveInformationsAction;
};


#endif /* PIMSETTINGEXPORTERWINDOW_H */

