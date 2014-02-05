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

#ifndef STORAGESERVICEDELETEDIALOG_H
#define STORAGESERVICEDELETEDIALOG_H


#include <KDialog>
#include "pimcommon_export.h"

class QTreeWidgetItem;
class QLabel;
namespace PimCommon {
class StorageServiceAbstract;
class StorageServiceTreeWidget;
class StorageServiceProgressIndicator;
class StorageServiceTreeWidgetItem;
class PIMCOMMON_EXPORT StorageServiceDeleteDialog : public KDialog
{
    Q_OBJECT
public:
    explicit StorageServiceDeleteDialog(PimCommon::StorageServiceAbstract *storage, QWidget *parent=0);
    ~StorageServiceDeleteDialog();

Q_SIGNALS:
    void deleteFileDone(const QString &, const QString &);
    void deleteFolderDone(const QString &, const QString &);

private slots:
    void slotItemActivated(QTreeWidgetItem *item, int column);

    void slotUpdatePixmap(const QPixmap &pix);
    void slotListFolderDone(const QString &serviceName, const QString &data);
    void slotActionFailed(const QString &serviceName, const QString &data);
    void slotItemDoubleClicked(QTreeWidgetItem *item, int);
    void slotDelete();

    void slotDeleteFolderDone(const QString &serviceName, const QString &filename);
    void slotDeleteFileDone(const QString &serviceName, const QString &filename);
    void slotRefreshList();

private:
    void deleteFile(StorageServiceTreeWidgetItem *storageServiceItem);
    void deleteFolder(StorageServiceTreeWidgetItem *storageServiceItem);


    void readConfig();
    void writeConfig();
    StorageServiceTreeWidget *mTreeWidget;
    PimCommon::StorageServiceAbstract *mStorage;
    PimCommon::StorageServiceProgressIndicator *mStorageServiceProgressIndicator;
    QLabel *mLabelProgressIncator;
};
}

#endif // STORAGESERVICEDELETEDIALOG_H
