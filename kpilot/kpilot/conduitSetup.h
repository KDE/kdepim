/***********************************************
** Source Dump From QtEZ http://qtez.commkey.net
** ---------------------------------------------
** Dumped: Wed Apr 15 00:37:56 1998
**     To: /home/pilone/Projects/KDE/kpilot-3.0/kpilot/testing//CConduitSetup.h
***********************************************/

#ifndef QTEZ_OUTP
 #define QTEZ_OUTP "QTEZ v0.44c"
#endif

#ifndef CCONDUITSETUP_H
#define CCONDUITSETUP_H

/* Library Includes */
#include <kdialogbase.h>
#include <qstrlist.h>

class QPushButton;
class QListBox;
class QLabel;

class
CConduitSetup : public KDialogBase
{
  Q_OBJECT

public:
  CConduitSetup(QWidget *parent=0, char *name=0);
	virtual ~CConduitSetup();
  
private:
  QLabel *label1;
  QLabel *label2;
  QListBox *fInstalledConduits;
  // QLabel *label3;
  QPushButton *fRemoveConduit;
  QPushButton *fDoneButton;
  QPushButton *fCancelButton;
  QListBox *fAvailableConduits;
  QPushButton *fInstallConduit;
  QPushButton *fSetupConduit;
  QStringList     fInstalledConduitNames;
  QStringList     fAvailableConduitNames;
  KProcess     fSetupConduitProcess;

  void fillLists();
  // Removes any installed items that aren't available
  void cleanupLists(const QStringList* available, QStringList* installed);
  void checkButtons() ;

protected:
public slots:
  void slotInstallConduit();
  void slotUninstallConduit();
  void slotSelectAvailable();
  void slotSelectInstalled();
  void slotSetupConduit();
protected slots:
  void slotOk();
  void slotCancel();
};
#endif
