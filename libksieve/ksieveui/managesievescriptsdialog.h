
#ifndef KSIEVE_KSIEVEUI_MANAGESIEVESCRIPTSDIALOG_H
#define KSIEVE_KSIEVEUI_MANAGESIEVESCRIPTSDIALOG_H

#include "ksieveui_export.h"

#include <qdialog.h>
#include <kurl.h>

#include <QMap>

class QButtonGroup;
class QTreeWidgetItem;
class KPushButton;
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
    void slotContextMenuRequested( QTreeWidgetItem*, QPoint position );
    void slotDoubleClicked( QTreeWidgetItem* );
    void slotSelectionChanged();
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

  private:
    bool serverHasError(QTreeWidgetItem *item) const;
    void killAllJobs( bool disconnect = false );
    void changeActiveScript( QTreeWidgetItem*, bool activate = true );

    /**
     * Adds a radio button to the specified item.
     */
    void addRadioButton( QTreeWidgetItem *item, const QString &text );

    /**
     * Turns the radio button for the specified item on or off.
     */
    void setRadioButtonState( QTreeWidgetItem *item, bool checked );

    /**
     * @return whether the specified item's radio button is checked or not
     */
    bool isRadioButtonChecked( QTreeWidgetItem *item ) const;

    /**
     * @return the text of the item. This is needed because the text is stored in the
     *         radio button, and not in the tree widget item.
     */
    QString itemText( QTreeWidgetItem *item ) const;

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

    TreeWidgetWithContextMenu* mListView;
    SieveEditor * mSieveEditor;
    QMap<KManageSieve::SieveJob*,QTreeWidgetItem*> mJobs;
    QMap<QTreeWidgetItem*,KUrl> mUrls;

    // Maps top-level items to their child which has the radio button selection
    QMap<QTreeWidgetItem*,QTreeWidgetItem*> mSelectedItems;

    // Maps the top-level tree widget items (the accounts) to a button group.
    // The button group is used for the radio buttons of the child items.
    QMap<QTreeWidgetItem*,QButtonGroup*> mButtonGroups;

    KUrl mCurrentURL;

    KPushButton *mNewScript;
    KPushButton *mEditScript;
    KPushButton *mDeleteScript;
    KPushButton *mDeactivateScript;

    bool mIsNewScript : 1;
    bool mWasActive : 1;
};

}

#endif
