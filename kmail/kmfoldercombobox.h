/* kmail folder-list combo-box
 * A specialized TQComboBox widget that refreshes its contents when
 * the folder list changes.
 */

#ifndef __KMFOLDERCOMBOBOX
#define __KMFOLDERCOMBOBOX

#include "kmfolder.h"

#include <tqcombobox.h>
#include <tqguardedptr.h>

class KMFolderComboBox : public QComboBox
{
  Q_OBJECT

public:
  KMFolderComboBox( TQWidget *parent = 0, char *name = 0 );
  KMFolderComboBox( bool rw, TQWidget *parent = 0, char *name = 0 );

  /** Select whether the outbox folder is shown.  Default is yes. */
  void showOutboxFolder(bool shown);

  /** Select whether the IMAP folders should be shown.  Default is yes. */
  void showImapFolders(bool shown);

  void setFolder( KMFolder *aFolder );
  void setFolder( const TQString &idString );
  KMFolder *getFolder();

public slots:
  /** Refresh list of folders in the combobox. */
  void refreshFolders();

private slots:
  void slotActivated(int index);

private:
  /** Create folder list using the folder manager. */
  void createFolderList(TQStringList *names,
                        TQValueList<TQGuardedPtr<KMFolder> > *folders);
  void init();

  TQGuardedPtr<KMFolder> mFolder;
  bool mOutboxShown;
  bool mImapShown;
  int mSpecialIdx;
};

#endif /* __KMFOLDERCOMBOBOX */
