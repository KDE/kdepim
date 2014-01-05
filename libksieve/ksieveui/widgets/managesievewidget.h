/*
  Copyright (c) 2014 Montel Laurent <montel.org>

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


#ifndef MANAGESIEVEWIDGET_H
#define MANAGESIEVEWIDGET_H
#include "ksieveui_export.h"

#include <KUrl>

#include <QWidget>
#include <QMap>

class QTreeWidgetItem;

namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {
class ManageSieveTreeView;
class KSIEVEUI_EXPORT ManageSieveWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ManageSieveWidget(QWidget *parent=0);
    ~ManageSieveWidget();

    ManageSieveTreeView *treeView() const;

protected:
    virtual bool refreshList() = 0;

private slots:
    void slotItemChanged(QTreeWidgetItem *item, int col);
    void slotContextMenuRequested(const QPoint &p);
    void slotDeactivateScript();
    void slotDeleteScript();
    void slotRefresh();

private:
    enum sieveServerStatus
    {
        SIEVE_SERVER_ERROR = Qt::UserRole +1,
        SIEVE_SERVER_CAPABILITIES = Qt::UserRole +2,
        SIEVE_SERVER_MODE = Qt::UserRole +3
    };
    bool serverHasError(QTreeWidgetItem *item) const;
    void killAllJobs();
    void clear();
    bool isFileNameItem(QTreeWidgetItem *item) const;
    bool itemIsActived(QTreeWidgetItem *item) const;
    void changeActiveScript(QTreeWidgetItem *item, bool activate);
    bool isProtectedName(const QString &name);

    QMap<KManageSieve::SieveJob*,QTreeWidgetItem*> mJobs;
    QMap<QTreeWidgetItem*,KUrl> mUrls;

    // Maps top-level items to their child which has the radio button selection
    QMap<QTreeWidgetItem*,QTreeWidgetItem*> mSelectedItems;
    ManageSieveTreeView *mTreeView;
    bool mClearAll : 1;
    bool mBlockSignal : 1;
};
}

#endif // MANAGESIEVEWIDGET_H
