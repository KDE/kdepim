/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef PIMSETTINGSBACKUPRESTORE_H
#define PIMSETTINGSBACKUPRESTORE_H

#include <QObject>
#include <QHash>
#include "utils.h"
class AbstractImportExportJob;
class ArchiveStorage;

class PimSettingsBackupRestore : public QObject
{
    Q_OBJECT
public:
    explicit PimSettingsBackupRestore(QWidget *parentWidget, QObject *parent = 0);
    ~PimSettingsBackupRestore();

    void backupStart(const QString &filename);
    void restoreStart(const QString &filename);

    void setStoredParameters(const QHash<Utils::AppsType, Utils::importExportParameters> &stored);

private slots:
    void slotJobFinished();

Q_SIGNALS:
    void addInfo(const QString &);
    void addEndLine();
    void updateActions(bool state);
    void addError(const QString &);
    void addTitle(const QString &);
    void jobFinished();
    void backupDone();
    void jobFailed();

private:
    enum Action {
        Backup,
        Restore
    };
    bool openArchive(const QString &filename, bool readWrite);
    void backupNextStep();
    void restoreNextStep();
    void backupFinished();
    void restoreFinished();
    void executeJob();

    QHash<Utils::AppsType, Utils::importExportParameters> mStored;
    QHash<Utils::AppsType, Utils::importExportParameters>::const_iterator mStoreIterator;
    Action mAction;
    AbstractImportExportJob *mImportExportData;
    ArchiveStorage *mArchiveStorage;
    QWidget *mParentWidget;
};

#endif // PIMSETTINGSBACKUPRESTORE_H
