/*
* Top Level window for KArm.
* Distributed under the GPL.
*/



#include <numeric>

#include <qkeycode.h>
#include <qpopupmenu.h>
#include <qptrlist.h>
#include <qstring.h>

#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>       // kapp
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kkeydialog.h>
#include <klocale.h>            // i18n
#include <kstatusbar.h>         // statusBar()
#include <kstdaction.h>

#include "kaccelmenuwatch.h"
#include "karmutility.h"
#include "mainwindow.h"
#include "preferences.h"
#include "print.h"
#include "timekard.h"
#include "task.h"
#include "taskview.h"
#include "tray.h"

MainWindow::MainWindow()
  : KMainWindow(0),
    _accel( new KAccel( this ) ),
    _watcher( new KAccelMenuWatch( _accel, this ) ),
    _taskView( new TaskView( this ) ),
    _totalSum( 0 ),
    _sessionSum( 0 )
{
  setCentralWidget( _taskView );
  // status bar
  startStatusBar();

  // setup PreferenceDialog.
  _preferences = Preferences::instance();

  // popup menus
  makeMenus();
  _watcher->updateMenus();

  // connections
  connect( _taskView, SIGNAL( totalTimesChanged( long, long ) ),
           this, SLOT( updateTime( long, long ) ) );
  connect( _taskView, SIGNAL( selectionChanged ( QListViewItem * )),
           this, SLOT(slotSelectionChanged()));
  connect( _taskView, SIGNAL( updateButtons() ),
           this, SLOT(slotSelectionChanged()));

  loadGeometry();

  // Setup context menu request handling
  connect( _taskView,
           SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int )),
           this,
           SLOT( contextMenuRequest( QListViewItem*, const QPoint&, int )));

  _tray = new KarmTray( this );

  connect( _tray, SIGNAL( quitSelected() ), SLOT( quit() ) );

  connect( _taskView, SIGNAL( timersActive() ), _tray, SLOT( startClock() ) );
  connect( _taskView, SIGNAL( timersActive() ), this,  SLOT( enableStopAll() ));
  connect( _taskView, SIGNAL( timersInactive() ), _tray, SLOT( stopClock() ) );
  connect( _taskView, SIGNAL( timersInactive() ),  this,  SLOT( disableStopAll()));
  connect( _taskView, SIGNAL( tasksChanged( QPtrList<Task> ) ),
                      _tray, SLOT( updateToolTip( QPtrList<Task> ) ));

  _taskView->load();

  // Everything that uses Preferences has been created now, we can let it
  // emit its signals
  _preferences->emitSignals();
  slotSelectionChanged();

}

void MainWindow::slotSelectionChanged()
{
  Task* item= _taskView->current_item();
  actionDelete->setEnabled(item);
  actionEdit->setEnabled(item);
  actionStart->setEnabled(item && !item->isRunning());
  actionStop->setEnabled(item && item->isRunning());
}

void MainWindow::timeLoggingChanged(bool on)
{
  actionAddComment->setEnabled( on );
}

void MainWindow::save()
{
  kdDebug() << i18n("Saving time data to disk.") << endl;
  _taskView->save();
  saveGeometry();
}

void MainWindow::quit()
{
  kapp->quit();
}


MainWindow::~MainWindow()
{
  kdDebug() << i18n("MainWindow::~MainWindows: Quitting karm.") << endl;
  _taskView->stopAllTimers();
  save();
}

void MainWindow::enableStopAll()
{
  actionStopAll->setEnabled(true);
}

void MainWindow::disableStopAll()
{
  actionStopAll->setEnabled(false);
}


/**
 * Calculate the sum of the session time and the total time for all
 * toplevel tasks and put it in the statusbar.
 */

void MainWindow::updateTime( long sessionDiff, long totalDiff )
{
  _sessionSum += sessionDiff;
  _totalSum   += totalDiff;

  updateStatusBar();
}

void MainWindow::updateStatusBar( )
{
  QString time;

  time = formatTime( _sessionSum );
  statusBar()->changeItem( i18n("Session: %1").arg(time), 0 );

  time = formatTime( _totalSum );
  statusBar()->changeItem( i18n("Total: %1" ).arg(time), 1);
}

void MainWindow::startStatusBar()
{
  statusBar()->insertItem( i18n("Session"), 0, 0, true );
  statusBar()->insertItem( i18n("Total" ), 1, 0, true );
}

void MainWindow::saveProperties( KConfig* )
{
  _taskView->stopAllTimers();
  _taskView->save();
}

void MainWindow::keyBindings()
{
  KKeyDialog::configureKeys( actionCollection(), xmlFile());
}

void MainWindow::startNewSession()
{
  _taskView->startNewSession();
}

void MainWindow::resetAllTimes()
{
  _taskView->resetTimeForAllTasks();
}

void MainWindow::makeMenus()
{
  KAction
    *actionKeyBindings,
    *actionNew,
    *actionNewSub;

  (void) KStdAction::quit(  this, SLOT( quit() ),  actionCollection());
  (void) KStdAction::print( this, SLOT( print() ), actionCollection());
  actionKeyBindings = KStdAction::keyBindings( this, SLOT( keyBindings() ),
      actionCollection() );
  actionPreferences = KStdAction::preferences(_preferences,
      SLOT(showDialog()),
      actionCollection() );
  (void) KStdAction::save( this, SLOT( save() ), actionCollection() );
  KAction* actionStartNewSession = new KAction( i18n("Start &New Session"),
      0,
      this,
      SLOT( startNewSession() ),
      actionCollection(),
      "start_new_session");
  KAction* actionResetAll = new KAction( i18n("&Reset All Times"),
      0,
      this,
      SLOT( resetAllTimes() ),
      actionCollection(),
      "reset_all_times");
  actionStart = new KAction( i18n("&Start"),
      QString::fromLatin1("1rightarrow"), Key_S,
      _taskView,
      SLOT( startCurrentTimer() ), actionCollection(),
      "start");
  actionStop = new KAction( i18n("S&top"),
      QString::fromLatin1("stop"), 0,
      _taskView,
      SLOT( stopCurrentTimer() ), actionCollection(),
      "stop");
  actionStopAll = new KAction( i18n("Stop &All Timers"),
      Key_Escape,
      _taskView,
      SLOT( stopAllTimers() ), actionCollection(),
      "stopAll");
  actionStopAll->setEnabled(false);

  actionNew = new KAction( i18n("&New..."),
      QString::fromLatin1("filenew"), CTRL+Key_N,
      _taskView,
      SLOT( newTask() ), actionCollection(),
      "new_task");
  actionNewSub = new KAction( i18n("New &Subtask..."),
      QString::fromLatin1("kmultiple"), CTRL+ALT+Key_N,
      _taskView,
      SLOT( newSubTask() ), actionCollection(),
      "new_sub_task");
  actionDelete = new KAction( i18n("&Delete"),
      QString::fromLatin1("editdelete"), Key_Delete,
      _taskView,
      SLOT( deleteTask() ), actionCollection(),
      "delete_task");
  actionEdit = new KAction( i18n("&Edit..."),
      QString::fromLatin1("edit"), CTRL + Key_E,
      _taskView,
      SLOT( editTask() ), actionCollection(),
      "edit_task");
  actionAddComment = new KAction( i18n("&Add Comment..."),
      QString::fromLatin1("document"),
      CTRL+ALT+Key_E,
      _taskView,
      SLOT( addCommentToTask() ),
      actionCollection(),
      "add_comment_to_task");
  actionMarkAsComplete = new KAction( i18n("&Mark as Complete..."),
      QString::fromLatin1("document"),
      CTRL+Key_M,
      _taskView,
      SLOT( markTaskAsComplete() ),
      actionCollection(),
      "mark_as_complete");
  actionClipTotals = new KAction( i18n("&Copy Totals to Clipboard"),
      QString::fromLatin1("klipper"),
      CTRL+Key_C,
      _taskView,
      SLOT( clipTotals() ),
      actionCollection(),
      "clip_totals");
  actionClipHistory = new KAction( i18n("Copy &History to Clipboard"),
      QString::fromLatin1("klipper"),
      CTRL+ALT+Key_C,
      _taskView,
      SLOT( clipHistory() ),
      actionCollection(),
      "clip_history");

  new KAction( i18n("Import &Legacy Flat File..."), 0,
      _taskView, SLOT(loadFromFlatFile()), actionCollection(),
      "import_flatfile");
  /*
  new KAction( i18n("Import E&vents"), 0,
                            _taskView,
                            SLOT( loadFromKOrgEvents() ), actionCollection(),
                            "import_korg_events");
  */

  createGUI( QString::fromLatin1("karmui.rc") );

  // Tool tops must be set after the createGUI.
  actionKeyBindings->setToolTip( i18n("Configure key bindings") );
  actionKeyBindings->setWhatsThis( i18n("This will let you configure key"
                                        "bindings which is specific to karm") );

  actionStartNewSession->setToolTip( i18n("Start a new session") );
  actionStartNewSession->setWhatsThis( i18n("This will reset the session time "
                                            "to 0 for all tasks, to start a "
                                            "new session, without affecting "
                                            "the totals.") );
  actionResetAll->setToolTip( i18n("Reset all times") );
  actionResetAll->setWhatsThis( i18n("This will reset the session and total "
                                     "time to 0 for all tasks, to restart from "
                                     "scratch.") );

  actionStart->setToolTip( i18n("Start timing for selected task") );
  actionStart->setWhatsThis( i18n("This will start timing for the selected "
                                  "task.\n"
                                  "It is even possible to time several tasks "
                                  "simultaneously.\n\n"
                                  "You may also start timing of a tasks by "
                                  "double clicking the left mouse "
                                  "button on a given task. This will, however, "
                                  "stop timing of other tasks."));

  actionStop->setToolTip( i18n("Stop timing of the selected task") );
  actionStop->setWhatsThis( i18n("Stop timing of the selected task") );

  actionStopAll->setToolTip( i18n("Stop all of the active timers") );
  actionStopAll->setWhatsThis( i18n("Stop all of the active timers") );

  actionNew->setToolTip( i18n("Create new top level task") );
  actionNew->setWhatsThis( i18n("This will create a new top level task.") );

  actionDelete->setToolTip( i18n("Delete selected task") );
  actionDelete->setWhatsThis( i18n("This will delete the selected task and "
                                   "all its subtasks.") );

  actionEdit->setToolTip( i18n("Edit name or times for selected task") );
  actionEdit->setWhatsThis( i18n("This will bring up a dialog box where you "
                                 "may edit the parameters for the selected "
                                 "task."));
  actionAddComment->setToolTip( i18n("Add a comment to a task") );
  actionAddComment->setWhatsThis( i18n("This will bring up a dialog box where "
                                       "you can add a comment to a task. The "
                                       "comment can for instance add information on what you "
                                       "are currently doing. The comment will "
                                       "be logged in the log file."));
  actionClipTotals->setToolTip(i18n("Copy task totals to clipboard"));
  actionClipHistory->setToolTip(i18n("Copy time card history to clipboard."));

  slotSelectionChanged();
}

void MainWindow::print()
{
  MyPrinter printer(_taskView);
  printer.print();
}

void MainWindow::loadGeometry()
{
  KConfig &config = *kapp->config();

  config.setGroup( QString::fromLatin1("Main Window Geometry") );
  int w = config.readNumEntry( QString::fromLatin1("Width"), 100 );
  int h = config.readNumEntry( QString::fromLatin1("Height"), 100 );
  w = QMAX( w, sizeHint().width() );
  h = QMAX( h, sizeHint().height() );
  resize(w, h);
}


void MainWindow::saveGeometry()
{
  KConfig &config = *KGlobal::config();
  config.setGroup( QString::fromLatin1("Main Window Geometry"));
  config.writeEntry( QString::fromLatin1("Width"), width());
  config.writeEntry( QString::fromLatin1("Height"), height());
  config.sync();
}

bool MainWindow::queryClose()
{
  if ( !kapp->sessionSaving() ) {
    hide();
    return false;
  }
  return KMainWindow::queryClose();
}

void MainWindow::contextMenuRequest( QListViewItem*, const QPoint& point, int )
{
    QPopupMenu* pop = dynamic_cast<QPopupMenu*>(
                          factory()->container( i18n( "task_popup" ), this ) );
    if ( pop )
      pop->popup( point );
}

#include "mainwindow.moc"
