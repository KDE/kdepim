/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef KSIEVE_KSIEVEUI_MANAGESIEVESCRIPTSDIALOG_H
#define KSIEVE_KSIEVEUI_MANAGESIEVESCRIPTSDIALOG_H

#include "ksieveui_export.h"

#include <QTreeWidget>
#include <qdialog.h>
#include <kurl.h>

#include <QMap>

class QTreeWidgetItem;
class KPushButton;

namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {
class SieveEditor;
class ManageSieveTreeView;
class KSIEVEUI_EXPORT ManageSieveScriptsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManageSieveScriptsDialog( QWidget *parent=0 );
    ~ManageSieveScriptsDialog();

    enum SieveEditorMode {
        NormalEditorMode = 0,
        Kep14EditorMode
    };

private slots:
    void slotRefresh();
    void slotGotList(KManageSieve::SieveJob *,bool success, const QStringList &listScript, const QString &activeScript);
    void slotContextMenuRequested( const QPoint& position );
    void slotDoubleClicked( QTreeWidgetItem* );
    void slotNewScript();
    void slotEditScript();
    void slotDeleteScript();
    void slotDeactivateScript();
    void slotGetResult( KManageSieve::SieveJob *, bool, const QString &, bool );
    void slotPutResult( KManageSieve::SieveJob *, bool );
    void slotPutResultDebug(KManageSieve::SieveJob *, bool success ,const QString &errorMsg);

    void slotSieveEditorOkClicked();
    void slotSieveEditorCancelClicked();
    void slotSieveEditorCheckSyntaxClicked();
    void slotUpdateButtons();
    void slotItemChanged(QTreeWidgetItem*, int);

private:
    bool isProtectedName(const QString &name);
    bool serverHasError(QTreeWidgetItem *item) const;
    void killAllJobs();
    void changeActiveScript( QTreeWidgetItem *, bool activate = true );

    /**
     * @return whether the specified item's radio button is checked or not
     */
    bool itemIsActived( QTreeWidgetItem *item ) const;

    /**
     * @return true if this tree widget item represents a sieve script, i.e. this item
     *              is not an account and not an error message.
     */
    bool isFileNameItem( QTreeWidgetItem *item ) const;

    /**
     * Remove everything from the tree widget and clear all caches.
     */
    void clear();

    void addFailedMessage( const QString &logEntry );
    void addOkMessage( const QString &logEntry );
    void addMessageEntry( const QString &errorMsg, const QColor &color );
    void updateButtons();
    void disableManagerScriptsDialog(bool disable);

private:
    enum sieveServerStatus
    {
        SIEVE_SERVER_ERROR = Qt::UserRole +1,
        SIEVE_SERVER_CAPABILITIES = Qt::UserRole +2,
        SIEVE_SERVER_MODE = Qt::UserRole +3
    };

    ManageSieveTreeView* mListView;
    SieveEditor * mSieveEditor;
    QMap<KManageSieve::SieveJob*,QTreeWidgetItem*> mJobs;
    QMap<QTreeWidgetItem*,KUrl> mUrls;

    // Maps top-level items to their child which has the radio button selection
    QMap<QTreeWidgetItem*,QTreeWidgetItem*> mSelectedItems;

    KUrl mCurrentURL;
    QStringList mCurrentCapabilities;

    KPushButton *mNewScript;
    KPushButton *mEditScript;
    KPushButton *mDeleteScript;
    KPushButton *mDeactivateScript;

    bool mIsNewScript : 1;
    bool mWasActive : 1;
    bool mBlockSignal : 1;
    bool mClearAll : 1;
};

}

#endif
