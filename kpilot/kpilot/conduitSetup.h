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

class QPushButton;
class QListBox;
class QLabel;

#define ENABLE_CMD_CS

class QListViewItem;
class KProcess;
class ListCategorizer;

class CConduitSetup : public KDialogBase
{
	Q_OBJECT

public:
	CConduitSetup(QWidget *parent, const char * name = 0);
	virtual ~CConduitSetup();


protected:
	QString findExecPath(const QListViewItem *) const;
	void writeInstalledConduits();
	void fillLists();

protected slots:
	void conduitExecuted(QListViewItem *);
	void setupDone(KProcess *);
	void slotOk();
	// void slotCancel();

private:
	void warnNoExec(const QListViewItem *);
	void warnSetupRunning();

	ListCategorizer *categories;
	QListViewItem *active,*available;
	KProcess *conduitSetup;
	QStringList conduitPaths;
} ;
#endif
