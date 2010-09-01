#ifndef __KMAIL__MANAGESIEVESCRIPTSDIALOG_H__
#define __KMAIL__MANAGESIEVESCRIPTSDIALOG_H__

#include <kdialogbase.h>
#include <kurl.h>
#include <tqmap.h>

class TQListView;
class TQCheckListItem;

namespace KMail {

class SieveJob;
class SieveEditor;

class ManageSieveScriptsDialog : public KDialogBase {
  Q_OBJECT
public:
  ManageSieveScriptsDialog( TQWidget * parent=0, const char * name=0 );
  ~ManageSieveScriptsDialog();

private slots:
  void slotRefresh();
  void slotItem( KMail::SieveJob *, const TQString &, bool );
  void slotResult( KMail::SieveJob *, bool, const TQString &, bool );
  void slotContextMenuRequested( TQListViewItem *, const TQPoint & );
  void slotDoubleClicked( TQListViewItem * );
  void slotSelectionChanged( TQListViewItem * );
  void slotNewScript();
  void slotEditScript();
  void slotDeactivateScript();
  void slotDeleteScript();
  void slotGetResult( KMail::SieveJob *, bool, const TQString &, bool );
  void slotPutResult( KMail::SieveJob *, bool );
  void slotSieveEditorOkClicked();
  void slotSieveEditorCancelClicked();

private:
  void killAllJobs();
  void changeActiveScript( TQCheckListItem *, bool activate = true );

private:
  TQListView * mListView;
  SieveEditor * mSieveEditor;
  TQMap<KMail::SieveJob*,TQCheckListItem*> mJobs;
  TQMap<TQCheckListItem*,KURL> mUrls;
  TQMap<TQCheckListItem*,TQCheckListItem*> mSelectedItems;
  TQCheckListItem * mContextMenuItem;
  KURL mCurrentURL;
  bool mWasActive : 1;
};

}

#endif /* __KMAIL__MANAGESIEVESCRIPTSDIALOG_H__ */

