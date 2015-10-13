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


#ifndef MANAGESIEVEWIDGET_H
#define MANAGESIEVEWIDGET_H
#include "ksieveui_export.h"

#include <KUrl>

#include <Solid/Networking>

#include <QWidget>
#include <QMap>

class QTreeWidgetItem;

namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {
class ManageSieveTreeView;
class ParseUserScriptJob;
class KSIEVEUI_EXPORT ManageSieveWidget : public QWidget
{
    Q_OBJECT
public:
    enum SieveEditorMode {
        NormalEditorMode = 0,
        Kep14EditorMode
    };
    explicit ManageSieveWidget(QWidget *parent=0);
    ~ManageSieveWidget();

    ManageSieveTreeView *treeView() const;
    void enableDisableActions(bool &newScriptAction, bool &editScriptAction, bool &deleteScriptAction, bool &desactivateScriptAction);

Q_SIGNALS:
    void updateButtons(QTreeWidgetItem *item);
    void newScript(const KUrl &u, const QStringList &currentCapabilities);
    void editScript(const KUrl &url, const QStringList &currentCapabilities);
    void scriptDeleted(const KUrl &u);

protected:
    virtual bool refreshList() = 0;

private Q_SLOTS:
    void slotItemChanged(QTreeWidgetItem *item, int col);
    void slotContextMenuRequested(const QPoint &p);
    void slotUpdateButtons();
    void slotDoubleClicked(QTreeWidgetItem *item);    
    void slotSystemNetworkStatusChanged(Solid::Networking::Status status);
    void slotCheckNetworkStatus();
    void setActiveScripts(ParseUserScriptJob *job);

public Q_SLOTS:
    void slotGotList(KManageSieve::SieveJob *job, bool success, const QStringList &listScript, const QString &activeScript);
    void slotNewScript();
    void slotEditScript();
    void slotDeleteScript();
    void slotDeactivateScript();
    void slotRefresh();

protected:
    QMap<KManageSieve::SieveJob*,QTreeWidgetItem*> mJobs;
    QMap<QTreeWidgetItem*,KUrl> mUrls;

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

    // Maps top-level items to their child which has the radio button selection
    QMap<QTreeWidgetItem*,QTreeWidgetItem*> mSelectedItems;
    ManageSieveTreeView *mTreeView;
    bool mClearAll : 1;
    bool mBlockSignal : 1;
};
}

#endif // MANAGESIEVEWIDGET_H
