
#ifndef KSIEVE_KSIEVEUI_MANAGESIEVESCRIPTSDIALOG_H
#define KSIEVE_KSIEVEUI_MANAGESIEVESCRIPTSDIALOG_H

#include "ksieveui_export.h"

#include <qdialog.h>
#include <kurl.h>

#include <QMap>

class QButtonGroup;
class QTreeWidgetItem;
class KPushButton;
class QTreeWidget;
namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {

class SieveEditor;
class TreeWidgetWithContextMenu;

class KSIEVEUI_EXPORT ManageSieveScriptsDialog : public QDialog
{
  Q_OBJECT

  public:
    explicit ManageSieveScriptsDialog( QWidget * parent=0, const char * name=0 );
    ~ManageSieveScriptsDialog();

  private slots:
    void slotRefresh( bool disconnectSignal = false );
    void slotItem( KManageSieve::SieveJob *, const QString &, bool );
    void slotResult( KManageSieve::SieveJob *, bool, const QString &, bool );
    void slotContextMenuRequested( const QPoint& position );
    void slotDoubleClicked( QTreeWidgetItem* );
    void slotNewScript();
    void slotEditScript();
    void slotDeleteScript();
    void slotDeactivateScript();
    void slotGetResult( KManageSieve::SieveJob *, bool, const QString &, bool );
    void slotPutResult( KManageSieve::SieveJob *, bool );
    void slotPutResultDebug(KManageSieve::SieveJob*,bool success ,const QString& errorMsg);

    void slotSieveEditorOkClicked();
    void slotSieveEditorCancelClicked();
    void slotSieveEditorCheckSyntaxClicked();
    void slotUpdateButtons();
    void slotItemChanged(QTreeWidgetItem*, int);

  private:
    bool serverHasError(QTreeWidgetItem *item) const;
    void killAllJobs( bool disconnect = false );
    void changeActiveScript( QTreeWidgetItem*, bool activate = true );

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
    void clear( bool disconnect = false );

    void addFailedMessage( const QString & logEntry );
    void addOkMessage( const QString & logEntry );
    void addMessageEntry( const QString & errorMsg, const QColor& color );
    void updateButtons();

  private:
    enum sieveServerStatus
    {
      SIEVE_SERVER_ERROR = Qt::UserRole +1
    };

    QTreeWidget* mListView;
    SieveEditor * mSieveEditor;
    QMap<KManageSieve::SieveJob*,QTreeWidgetItem*> mJobs;
    QMap<QTreeWidgetItem*,KUrl> mUrls;

    // Maps top-level items to their child which has the radio button selection
    QMap<QTreeWidgetItem*,QTreeWidgetItem*> mSelectedItems;

    KUrl mCurrentURL;

    KPushButton *mNewScript;
    KPushButton *mEditScript;
    KPushButton *mDeleteScript;
    KPushButton *mDeactivateScript;

    bool mIsNewScript : 1;
    bool mWasActive : 1;
    bool mBlockSignal : 1;
};

}

#endif
