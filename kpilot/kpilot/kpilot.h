/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

// REVISION HISTORY 
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place,
//		added global debug_level variable and some #defines
//		for its meaning.

#ifndef __KPILOT_H
#define __KPILOT_H

#include <qapp.h>
#include <qmenubar.h>
#include <qlist.h>
#include <qpopmenu.h>

#include <kfm.h>
#include <kapp.h>
#include <klocale.h>
#include <ktopwidget.h>
#include <kprogress.h>
#include <kprocess.h>

#include "fileInstallWidget.h"
#include "memoWidget.h"

// Global variables that all conduits and apps should respect.
// In old C-style:
//
//
extern int debug_level;
#define NO_DEBUG	(0)
#define UI_ACTIONS 	(4)
#define TEDIOUS		(16)

#define FUNCTIONSETUP	static char *fname=__FUNCTION__; \
			if (debug_level>TEDIOUS) { cerr << fname << \
				": Entered\t(" << __FILE__ << ':' << \
				__LINE__ << ")\n"; } 

// Some layout macros
//
// SPACING is a generic distance between visual elements;
// 10 seems reasonably good even at high resolutions.
//
// Give RIGHT and BELOW a QWidget. In all likelihood
// these will disappear soon with a new layout style in
// KPilot 3.2.
//
#define SPACING		(10)
#define BELOW(a)	a->y()+a->height()+SPACING
#define RIGHT(a)	a->x()+a->width()+SPACING







class KPilotInstaller : public KTopLevelWidget
    {
    Q_OBJECT

    public:
    KPilotInstaller();
    KPilotInstaller(QStrList& fileList);
    ~KPilotInstaller();

    void testDir(QString name);
    bool getQuitAfterCopyComplete() const { return fQuitAfterCopyComplete; }
    // Adds 'name' to the pull down menu of components
    void addComponentPage(QWidget* widget, QString name);

    protected:
      void closeEvent(QCloseEvent *e);
      void setQuitAfterCopyComplete(bool quit) { fQuitAfterCopyComplete = quit; }
      QWidget* getManagingWidget() { return fManagingWidget; }
      
      // Not sure if this is the way to go or not... might want to make the link
      // persist...
      void initPilotLink();
      void initStatusLink(); // Seperate so the components can be initialized
      KPilotLink* getPilotLink() { return fPilotLink; }
      void destroyPilotLink() { if (fPilotLink) {delete fPilotLink; fPilotLink = 0L; /* delete fLinkProcess; fLinkProcess = 0L; */ } }
      void doRestore();
      void doBackup();

private:
      void initMenu();
      void initStatusBar();
      void initToolBar();
      void setupWidget();
      void initComponents();
      
      static const int ID_FILE_QUIT;
      static const int ID_FILE_SETTINGS;
      static const int ID_FILE_BACKUP;
      static const int ID_FILE_RESTORE;
      static const int ID_HELP_ABOUT;
      static const int ID_HELP_HELP;
      static const int ID_CONDUITS_ENABLE;
      static const int ID_CONDUITS_SETUP;
      static const int ID_COMBO;
      
      KMenuBar*       fMenuBar;
      KStatusBar*     fStatusBar;
      KToolBar*       fToolBar;
      bool            fQuitAfterCopyComplete; // Used for GUI-less interface
      QWidget*        fManagingWidget;
      QList<PilotComponent>  fPilotComponentList; // Has the widgets/components...
      QList<QWidget>  fVisibleWidgetList;
      KPilotLink*     fPilotLink;
      KSocket*        fPilotCommandSocket;
      KSocket*        fPilotStatusSocket;
      bool            fKillDaemonOnExit;
      char            fLinkCommand[10000];
      int             fLastWidgetSelected;

 protected slots:
      void menuCallback(int);
      void quit();
      void doHotSync();
      void fileInstalled(int which);
      void slotModeSelected(int selected);
      void slotSyncDone(KProcess* which);
      void slotDaemonStatus(KSocket*);

    signals:
    void modeSelected(int selected);
    };



 
#endif
