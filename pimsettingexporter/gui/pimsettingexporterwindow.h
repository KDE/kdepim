/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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
class KRecentFilesAction;
class QAction;
class QCommandLineParser;
class PimSettingsBackupRestoreUI;

class PimSettingExporterWindow: public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit PimSettingExporterWindow(QWidget *parent = Q_NULLPTR);
    ~PimSettingExporterWindow();
    void handleCommandLine(const QCommandLineParser &parser);

private Q_SLOTS:
    void slotBackupData();
    void slotRestoreData();
    void slotAddInfo(const QString &info);
    void slotAddError(const QString &info);
    void slotAddTitle(const QString &info);
    void slotAddEndLine();
    void slotSaveLog();
    void slotShowStructureInfos();
    void slotRestoreFile(const QUrl &url);
    void slotShowArchiveInformations();
    void slotUpdateActions(bool inAction);
    void slotShowBackupFinishDialogInformation();
    void slotJobFailed();
    void slotJobFinished();
    void slotShowCurrentArchiveInformations();
    void slotAddResourceToSync(const QString &identifier);
    void slotFullSyncFinished();
    void slotFullSyncInstanceDone(const QString &identifier);
    void slotFullSyncInstanceFailed(const QString &identifier);
    void slotRestoreDone();
private:
    void initializeBackupRestoreUi();
    void backupData(const QString &filename, const QString &templateFile = QString());
    void loadData(const QString &filename, const QString &templateFile = QString());
    void setupActions(bool canZipFile);
    void showFinishInformation();
    LogWidget *mLogWidget;
    KRecentFilesAction *mRecentFilesAction;
    QAction *mBackupAction;
    QAction *mRestoreAction;
    QAction *mSaveLogAction;
    QAction *mArchiveStructureInfo;
    QAction *mShowArchiveInformationsAction;
    QAction *mShowArchiveInformationsAboutCurrentArchiveAction;
    PimSettingsBackupRestoreUI *mPimSettingsBackupRestoreUI;
    QString mLastArchiveFileName;
    QStringList mNeedToSyncResources;
};

#endif /* PIMSETTINGEXPORTERWINDOW_H */

