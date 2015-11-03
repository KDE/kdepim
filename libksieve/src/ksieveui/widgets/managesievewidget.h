/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include <QUrl>

#include <QWidget>
#include <QMap>

class QTreeWidgetItem;

namespace KManageSieve
{
class SieveJob;
}

namespace KSieveUi
{
class ManageSieveTreeView;
class ManageSieveWidgetPrivate;
class KSIEVEUI_EXPORT ManageSieveWidget : public QWidget
{
    Q_OBJECT
public:
    enum SieveEditorMode {
        NormalEditorMode = 0,
        Kep14EditorMode
    };
    explicit ManageSieveWidget(QWidget *parent = Q_NULLPTR);
    ~ManageSieveWidget();

    ManageSieveTreeView *treeView() const;
    void enableDisableActions(bool &newScriptAction, bool &editScriptAction, bool &deleteScriptAction, bool &desactivateScriptAction);

Q_SIGNALS:
    void updateButtons(QTreeWidgetItem *item);
    void newScript(const QUrl &u, const QStringList &currentCapabilities);
    void editScript(const QUrl &url, const QStringList &currentCapabilities);
    void scriptDeleted(const QUrl &u);
    void serverSieveFound(bool imapFound);

protected:
    virtual bool refreshList() = 0;

private Q_SLOTS:
    void slotItemChanged(QTreeWidgetItem *item, int col);
    void slotContextMenuRequested(const QPoint &p);
    void slotUpdateButtons();
    void slotDoubleClicked(QTreeWidgetItem *item);
    void slotSystemNetworkOnlineStateChanged(bool state);
    void slotCheckNetworkStatus();

    void slotCancelFetch();
public Q_SLOTS:
    void slotGotList(KManageSieve::SieveJob *job, bool success, const QStringList &listScript, const QString &activeScript);
    void slotNewScript();
    void slotEditScript();
    void slotDeleteScript();
    void slotDeactivateScript();
    void slotRefresh();

protected:
    QMap<KManageSieve::SieveJob *, QTreeWidgetItem *> mJobs;
    QMap<QTreeWidgetItem *, QUrl> mUrls;

private:
    enum sieveServerStatus {
        SIEVE_SERVER_ERROR = Qt::UserRole + 1,
        SIEVE_SERVER_CAPABILITIES = Qt::UserRole + 2,
        SIEVE_SERVER_MODE = Qt::UserRole + 3
    };
    bool serverHasError(QTreeWidgetItem *item) const;
    void killAllJobs();
    void clear();
    bool isFileNameItem(QTreeWidgetItem *item) const;
    bool itemIsActived(QTreeWidgetItem *item) const;
    void changeActiveScript(QTreeWidgetItem *item, bool activate);
    bool isProtectedName(const QString &name);

    ManageSieveWidgetPrivate *const d;
};
}

#endif // MANAGESIEVEWIDGET_H
