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


#include <QDialog>
#include <KConfigGroup>
#include "pimcommon_export.h"
#include "storageservice/widgets/storageservicetreewidget.h"

class QTreeWidgetItem;
class QLabel;
class QMenu;
class QPushButton;
namespace PimCommon {
class StorageServiceAbstract;
class StorageServiceProgressIndicator;
class StorageServiceTreeWidgetItem;
class StorageServiceDeleteTreeWidget;
class PIMCOMMON_EXPORT StorageServiceDeleteDialog : public QDialog
{
    Q_OBJECT
public:
    enum DeleteType {
        DeleteAll = 0,
        DeleteFiles,
        DeleteFolders
    };

    explicit StorageServiceDeleteDialog(PimCommon::StorageServiceDeleteDialog::DeleteType type, PimCommon::StorageServiceAbstract *storage, QWidget *parent=0);
    ~StorageServiceDeleteDialog();

Q_SIGNALS:
    void deleteFileDone(const QString &, const QString &);
    void deleteFolderDone(const QString &, const QString &);

private slots:
    void slotItemActivated(QTreeWidgetItem *item, int column);

    void slotUpdatePixmap(const QPixmap &pix);
    void slotListFolderDone(const QString &serviceName, const QVariant &data);
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
    DeleteType mDeleteType;
    StorageServiceDeleteTreeWidget *mTreeWidget;
    PimCommon::StorageServiceAbstract *mStorage;
    PimCommon::StorageServiceProgressIndicator *mStorageServiceProgressIndicator;
    QLabel *mLabelProgressIncator;
    QPushButton *mUser1Button;
};

class StorageServiceDeleteTreeWidget : public PimCommon::StorageServiceTreeWidget
{
    Q_OBJECT
public:
    explicit StorageServiceDeleteTreeWidget(PimCommon::StorageServiceDeleteDialog::DeleteType type, PimCommon::StorageServiceAbstract *storageService, QWidget *parent=0);

    PimCommon::StorageServiceDeleteDialog::DeleteType deleteType() const;

Q_SIGNALS:
    void deleteFileFolder();

protected:
    virtual void createMenuActions(QMenu *menu);

private:
    PimCommon::StorageServiceDeleteDialog::DeleteType mDeleteType;
};

}

#endif // STORAGESERVICEDELETEDIALOG_H
