/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef STORAGESERVICEDOWNLOADDIALOG_H
#define STORAGESERVICEDOWNLOADDIALOG_H

#include <QDialog>
#include <KConfigGroup>
#include "pimcommon_export.h"
#include "storageservice/widgets/storageservicetreewidget.h"

class QTreeWidgetItem;
class QLabel;
namespace PimCommon
{
class StorageServiceAbstract;
class StorageServiceProgressWidget;
class StorageServiceProgressIndicator;
class StorageServiceTreeWidgetItem;
class StorageServiceDownloadTreeWidget;
class PIMCOMMON_EXPORT StorageServiceDownloadDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StorageServiceDownloadDialog(PimCommon::StorageServiceAbstract *storage, QWidget *parent = Q_NULLPTR);
    ~StorageServiceDownloadDialog();

    void setDefaultDownloadPath(const QString &path);

protected:
    void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

private slots:
    void slotItemActivated(QTreeWidgetItem *item, int column);
    void slotDownloadFile();

    void slotDownfileDone(const QString &serviceName, const QString &filename);

    void slotDownfileFailed(const QString &serviceName, const QString &filename);
    void slotUpdatePixmap(const QPixmap &pix);
    void slotListFolderDone(const QString &serviceName, const QVariant &data);
    void slotActionFailed(const QString &serviceName, const QString &data);
    void slotUploadDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total);
    void slotItemDoubleClicked(QTreeWidgetItem *item, int);
private:
    void reenableDialog();
    void downloadItem(PimCommon::StorageServiceTreeWidgetItem *item);
    void readConfig();
    void writeConfig();
    QString mDefaultDownloadPath;
    StorageServiceDownloadTreeWidget *mTreeWidget;
    PimCommon::StorageServiceAbstract *mStorage;
    PimCommon::StorageServiceProgressWidget *mProgressWidget;
    PimCommon::StorageServiceProgressIndicator *mStorageServiceProgressIndicator;
    QLabel *mLabelProgressIncator;
    QPushButton *mUser1Button;
    QPushButton *mCloseButton;
};

class StorageServiceDownloadTreeWidget : public PimCommon::StorageServiceTreeWidget
{
    Q_OBJECT
public:
    explicit StorageServiceDownloadTreeWidget(PimCommon::StorageServiceAbstract *storageService, QWidget *parent = Q_NULLPTR);

Q_SIGNALS:
    void downloadFile();

protected:
    virtual void createMenuActions(QMenu *menu);
};

}

#endif // STORAGESERVICEDOWNLOADDIALOG_H
