
#include "kaccelmenuwatch.h"
#include "karm_part.h"
#include "karmerrors.h"
#include "task.h"
#include "preferences.h"
#include "tray.h"
#include "version.h"
#include <kaccel.h>

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <klocale.h>

#include <tqfile.h>
#include <tqtextstream.h>
#include <tqmultilineedit.h>
#include <tqpopupmenu.h>
#include "mainwindow.h"

karmPart::karmPart( TQWidget *parentWidget, const char *widgetName,
                                  TQObject *parent, const char *name )
    : DCOPObject ( "KarmDCOPIface" ), KParts::ReadWritePart(parent, name), 
    _accel     ( new KAccel( parentWidget ) ),
    _watcher   ( new KAccelMenuWatch( _accel, parentWidget ) )
{
    // we need an instance
    setInstance( karmPartFactory::instance() );

    // this should be your custom internal widget
    _taskView = new TaskView( parentWidget, widgetName );

    // setup PreferenceDialog.
    _preferences = Preferences::instance();

   // notify the part that this is our internal widget
    setWidget(_taskView);

    // create our actions
    KStdAction::open(this, TQT_SLOT(fileOpen()), actionCollection());
    KStdAction::saveAs(this, TQT_SLOT(fileSaveAs()), actionCollection());
    KStdAction::save(this, TQT_SLOT(save()), actionCollection());

    makeMenus();

  _watcher->updateMenus();

  // connections

  connect( _taskView, TQT_SIGNAL( totalTimesChanged( long, long ) ),
           this, TQT_SLOT( updateTime( long, long ) ) );
  connect( _taskView, TQT_SIGNAL( selectionChanged ( TQListViewItem * )),
           this, TQT_SLOT(slotSelectionChanged()));
  connect( _taskView, TQT_SIGNAL( updateButtons() ),
           this, TQT_SLOT(slotSelectionChanged()));

  // Setup context menu request handling
  connect( _taskView,
           TQT_SIGNAL( contextMenuRequested( TQListViewItem*, const TQPoint&, int )),
           this,
           TQT_SLOT( contextMenuRequest( TQListViewItem*, const TQPoint&, int )));

  _tray = new KarmTray( this );

  connect( _tray, TQT_SIGNAL( quitSelected() ), TQT_SLOT( quit() ) );

  connect( _taskView, TQT_SIGNAL( timersActive() ), _tray, TQT_SLOT( startClock() ) );
  connect( _taskView, TQT_SIGNAL( timersActive() ), this,  TQT_SLOT( enableStopAll() ));
  connect( _taskView, TQT_SIGNAL( timersInactive() ), _tray, TQT_SLOT( stopClock() ) );
  connect( _taskView, TQT_SIGNAL( timersInactive() ),  this,  TQT_SLOT( disableStopAll()));
  connect( _taskView, TQT_SIGNAL( tasksChanged( TQPtrList<Task> ) ),
                      _tray, TQT_SLOT( updateToolTip( TQPtrList<Task> ) ));

  _taskView->load();

  // Everything that uses Preferences has been created now, we can let it
  // emit its signals
  _preferences->emitSignals();
  slotSelectionChanged();

    // set our XML-UI resource file
    setXMLFile("karmui.rc");

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);
}

karmPart::~karmPart()
{
}

void karmPart::slotSelectionChanged()
{
  Task* item= _taskView->current_item();
  actionDelete->setEnabled(item);
  actionEdit->setEnabled(item);
  actionStart->setEnabled(item && !item->isRunning() && !item->isComplete());
  actionStop->setEnabled(item && item->isRunning());
  actionMarkAsComplete->setEnabled(item && !item->isComplete());
  actionMarkAsIncomplete->setEnabled(item && item->isComplete());
}

void karmPart::makeMenus()
{
  KAction
    *actionKeyBindings,
    *actionNew,
    *actionNewSub;

  (void) KStdAction::quit(  this, TQT_SLOT( quit() ),  actionCollection());
  (void) KStdAction::print( this, TQT_SLOT( print() ), actionCollection());
  actionKeyBindings = KStdAction::keyBindings( this, TQT_SLOT( keyBindings() ),
      actionCollection() );
  actionPreferences = KStdAction::preferences(_preferences,
      TQT_SLOT(showDialog()),
      actionCollection() );
  (void) KStdAction::save( this, TQT_SLOT( save() ), actionCollection() );
  KAction* actionStartNewSession = new KAction( i18n("Start &New Session"),
      0,
      this,
      TQT_SLOT( startNewSession() ),
      actionCollection(),
      "start_new_session");
  KAction* actionResetAll = new KAction( i18n("&Reset All Times"),
      0,
      this,
      TQT_SLOT( resetAllTimes() ),
      actionCollection(),
      "reset_all_times");
  actionStart = new KAction( i18n("&Start"),
      TQString::fromLatin1("1rightarrow"), Key_S,
      _taskView,
      TQT_SLOT( startCurrentTimer() ), actionCollection(),
      "start");
  actionStop = new KAction( i18n("S&top"),
      TQString::fromLatin1("stop"), 0,
      _taskView,
      TQT_SLOT( stopCurrentTimer() ), actionCollection(),
      "stop");
  actionStopAll = new KAction( i18n("Stop &All Timers"),
      Key_Escape,
      _taskView,
      TQT_SLOT( stopAllTimers() ), actionCollection(),
      "stopAll");
  actionStopAll->setEnabled(false);

  actionNew = new KAction( i18n("&New..."),
      TQString::fromLatin1("filenew"), CTRL+Key_N,
      _taskView,
      TQT_SLOT( newTask() ), actionCollection(),
      "new_task");
  actionNewSub = new KAction( i18n("New &Subtask..."),
      TQString::fromLatin1("kmultiple"), CTRL+ALT+Key_N,
      _taskView,
      TQT_SLOT( newSubTask() ), actionCollection(),
      "new_sub_task");
  actionDelete = new KAction( i18n("&Delete"),
      TQString::fromLatin1("editdelete"), Key_Delete,
      _taskView,
      TQT_SLOT( deleteTask() ), actionCollection(),
      "delete_task");
  actionEdit = new KAction( i18n("&Edit..."),
      TQString::fromLatin1("edit"), CTRL + Key_E,
      _taskView,
      TQT_SLOT( editTask() ), actionCollection(),
      "edit_task");
//  actionAddComment = new KAction( i18n("&Add Comment..."),
//      TQString::fromLatin1("document"),
//      CTRL+ALT+Key_E,
//      _taskView,
//      TQT_SLOT( addCommentToTask() ),
//      actionCollection(),
//      "add_comment_to_task");
  actionMarkAsComplete = new KAction( i18n("&Mark as Complete"),
      TQString::fromLatin1("document"),
      CTRL+Key_M,
      _taskView,
      TQT_SLOT( markTaskAsComplete() ),
      actionCollection(),
      "mark_as_complete");
  actionMarkAsIncomplete = new KAction( i18n("&Mark as Incomplete"),
      TQString::fromLatin1("document"),
      CTRL+Key_M,
      _taskView,
      TQT_SLOT( markTaskAsIncomplete() ),
      actionCollection(),
      "mark_as_incomplete");
  actionClipTotals = new KAction( i18n("&Copy Totals to Clipboard"),
      TQString::fromLatin1("klipper"),
      CTRL+Key_C,
      _taskView,
      TQT_SLOT( clipTotals() ),
      actionCollection(),
      "clip_totals");
  actionClipHistory = new KAction( i18n("Copy &History to Clipboard"),
      TQString::fromLatin1("klipper"),
      CTRL+ALT+Key_C,
      _taskView,
      TQT_SLOT( clipHistory() ),
      actionCollection(),
      "clip_history");

  new KAction( i18n("Import &Legacy Flat File..."), 0,
      _taskView, TQT_SLOT(loadFromFlatFile()), actionCollection(),
      "import_flatfile");
  new KAction( i18n("&Export to CSV File..."), 0,
      _taskView, TQT_SLOT(exportcsvFile()), actionCollection(),
      "export_csvfile");
  new KAction( i18n("Export &History to CSV File..."), 0,
      this, TQT_SLOT(exportcsvHistory()), actionCollection(),
      "export_csvhistory");
  new KAction( i18n("Import Tasks From &Planner..."), 0,
      _taskView, TQT_SLOT(importPlanner()), actionCollection(),
      "import_planner");  
  new KAction( i18n("Configure KArm..."), 0,
      _preferences, TQT_SLOT(showDialog()), actionCollection(),
      "configure_karm");  

/*
  new KAction( i18n("Import E&vents"), 0,
                            _taskView,
                            TQT_SLOT( loadFromKOrgEvents() ), actionCollection(),
                            "import_korg_events");
  */

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
  //actionAddComment->setToolTip( i18n("Add a comment to a task") );
  //actionAddComment->setWhatsThis( i18n("This will bring up a dialog box where "
  //                                     "you can add a comment to a task. The "
  //                                     "comment can for instance add information on what you "
  //                                     "are currently doing. The comment will "
  //                                     "be logged in the log file."));
  actionClipTotals->setToolTip(i18n("Copy task totals to clipboard"));
  actionClipHistory->setToolTip(i18n("Copy time card history to clipboard."));

  slotSelectionChanged();
}

void karmPart::setReadWrite(bool rw)
{
    // notify your internal widget of the read-write state
    if (rw)
        connect(_taskView, TQT_SIGNAL(textChanged()),
                this,     TQT_SLOT(setModified()));
    else
    {
        disconnect(_taskView, TQT_SIGNAL(textChanged()),
                   this,     TQT_SLOT(setModified()));
    }

    ReadWritePart::setReadWrite(rw);
}

void karmPart::setModified(bool modified)
{
    // get a handle on our Save action and make sure it is valid
    KAction *save = actionCollection()->action(KStdAction::stdName(KStdAction::Save));
    if (!save)
        return;

    // if so, we either enable or disable it based on the current
    // state
    if (modified)
        save->setEnabled(true);
    else
        save->setEnabled(false);

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);
}

bool karmPart::openFile()
{
    // m_file is always local so we can use TQFile on it
    _taskView->load(m_file);

    // just for fun, set the status bar
    emit setStatusBarText( m_url.prettyURL() );

    return true;
}

bool karmPart::saveFile()
{
    // if we aren't read-write, return immediately
    if (isReadWrite() == false)
        return false;

    // m_file is always local, so we use QFile
    TQFile file(m_file);
    if (file.open(IO_WriteOnly) == false)
        return false;

    // use TQTextStream to dump the text to the file
    TQTextStream stream(&file);

    file.close();

    return true;
}

void karmPart::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    TQString file_name = KFileDialog::getOpenFileName();

    if (file_name.isEmpty() == false)
        openURL(file_name);
}

void karmPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    TQString file_name = KFileDialog::getSaveFileName();
    if (file_name.isEmpty() == false)
        saveAs(file_name);
}


// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance*  karmPartFactory::s_instance = 0L;
KAboutData* karmPartFactory::s_about = 0L;

karmPartFactory::karmPartFactory()
    : KParts::Factory()
{
}

karmPartFactory::~karmPartFactory()
{
    delete s_instance;
    delete s_about;

    s_instance = 0L;
}

KParts::Part* karmPartFactory::createPartObject( TQWidget *parentWidget, const char *widgetName,
                                                        TQObject *parent, const char *name,
                                                        const char *classname, const TQStringList &args )
{
    // Create an instance of our Part
    karmPart* obj = new karmPart( parentWidget, widgetName, parent, name );

    // See if we are to be read-write or not
    if (TQCString(classname) == "KParts::ReadOnlyPart")
        obj->setReadWrite(false);

    return obj;
}

KInstance* karmPartFactory::instance()
{
    if( !s_instance )
    {
        s_about = new KAboutData("karmpart", I18N_NOOP("karmPart"), "0.1");
        s_about->addAuthor("Thorsten Staerk", 0, "thorsten@staerk.de");
        s_instance = new KInstance(s_about);
    }
    return s_instance;
}

extern "C"
{
    KDE_EXPORT void* init_libkarmpart()
    {
	KGlobal::locale()->insertCatalogue("karm");
        return new karmPartFactory;
    }
}

void karmPart::contextMenuRequest( TQListViewItem*, const TQPoint& point, int )
{
    TQPopupMenu* pop = dynamic_cast<TQPopupMenu*>(
                          factory()->container( i18n( "task_popup" ), this ) );
    if ( pop )
      pop->popup( point );
}

//----------------------------------------------------------------------------
//
//                       D C O P   I N T E R F A C E
//
//----------------------------------------------------------------------------

TQString karmPart::version() const
{
  return KARM_VERSION;
}

TQString karmPart::deletetodo()
{
  _taskView->deleteTask();
  return "";
}

bool karmPart::getpromptdelete()
{
  return _preferences->promptDelete();
}

TQString karmPart::setpromptdelete( bool prompt )
{
  _preferences->setPromptDelete( prompt );
  return "";
}

TQString karmPart::taskIdFromName( const TQString &taskname ) const
{
  TQString rval = "";

  Task* task = _taskView->first_child();
  while ( rval.isEmpty() && task )
  {
    rval = _hasTask( task, taskname );
    task = task->nextSibling();
  }
  
  return rval;
}

void karmPart::quit()
{
  // TODO: write something for kapp->quit();
}

bool karmPart::save()
{
  kdDebug(5970) << "Saving time data to disk." << endl;
  TQString err=_taskView->save();  // untranslated error msg.
  // TODO:
  /* if (err.isEmpty()) statusBar()->message(i18n("Successfully saved tasks and history"),1807);
  else statusBar()->message(i18n(err.ascii()),7707); // no msgbox since save is called when exiting */
  return true;
}

int karmPart::addTask( const TQString& taskname ) 
{
  DesktopList desktopList;
  TQString uid = _taskView->addTask( taskname, 0, 0, desktopList );
  kdDebug(5970) << "MainWindow::addTask( " << taskname << " ) returns " << uid << endl;
  if ( uid.length() > 0 ) return 0;
  else
  {
    // We can't really tell what happened, b/c the resource framework only
    // returns a boolean.
    return KARM_ERR_GENERIC_SAVE_FAILED;
  }
}

TQString karmPart::setPerCentComplete( const TQString& taskName, int perCent )
{
  int index = 0;
  TQString err="no such task";
  for (int i=0; i<_taskView->count(); i++)
  {
    if ((_taskView->item_at_index(i)->name()==taskName))
    {
      index=i;
      if (err==TQString::null) err="task name is abigious";
      if (err=="no such task") err=TQString::null;
    }
  }
  if (err==TQString::null) 
  {
    _taskView->item_at_index(index)->setPercentComplete( perCent, _taskView->storage() );
  }
  return err;
}

int karmPart::bookTime
( const TQString& taskId, const TQString& datetime, long minutes )
{
  int rval = 0;
  TQDate startDate;
  TQTime startTime;
  TQDateTime startDateTime;
  Task *task, *t;

  if ( minutes <= 0 ) rval = KARM_ERR_INVALID_DURATION;

  // Find task
  task = _taskView->first_child();
  t = NULL;
  while ( !t && task )
  {
    t = _hasUid( task, taskId );
    task = task->nextSibling();
  }
  if ( t == NULL ) rval = KARM_ERR_UID_NOT_FOUND;

  // Parse datetime
  if ( !rval ) 
  {
    startDate = TQDate::fromString( datetime, Qt::ISODate );
    if ( datetime.length() > 10 )  // "YYYY-MM-DD".length() = 10
    {
      startTime = TQTime::fromString( datetime, Qt::ISODate );
    }
    else startTime = TQTime( 12, 0 );
    if ( startDate.isValid() && startTime.isValid() )
    {
      startDateTime = TQDateTime( startDate, startTime );
    }
    else rval = KARM_ERR_INVALID_DATE;

  }

  // Update task totals (session and total) and save to disk
  if ( !rval )
  {
    t->changeTotalTimes( t->sessionTime() + minutes, t->totalTime() + minutes );
    if ( ! _taskView->storage()->bookTime( t, startDateTime, minutes * 60 ) )
    {
      rval = KARM_ERR_GENERIC_SAVE_FAILED;
    }
  }

  return rval;
}

// There was something really bad going on with DCOP when I used a particular
// argument name; if I recall correctly, the argument name was errno.
TQString karmPart::getError( int mkb ) const
{
  if ( mkb <= KARM_MAX_ERROR_NO ) return m_error[ mkb ];
  else return i18n( "Invalid error number: %1" ).arg( mkb );
}

int karmPart::totalMinutesForTaskId( const TQString& taskId )
{
  int rval = 0;
  Task *task, *t;
  
  kdDebug(5970) << "MainWindow::totalTimeForTask( " << taskId << " )" << endl;

  // Find task
  task = _taskView->first_child();
  t = NULL;
  while ( !t && task )
  {
    t = _hasUid( task, taskId );
    task = task->nextSibling();
  }
  if ( t != NULL ) 
  {
    rval = t->totalTime();
    kdDebug(5970) << "MainWindow::totalTimeForTask - task found: rval = " << rval << endl;
  }
  else 
  {
    kdDebug(5970) << "MainWindow::totalTimeForTask - task not found" << endl;
    rval = KARM_ERR_UID_NOT_FOUND;
  }

  return rval;
}

TQString karmPart::_hasTask( Task* task, const TQString &taskname ) const
{
  TQString rval = "";
  if ( task->name() == taskname ) 
  {
    rval = task->uid();
  }
  else
  {
    Task* nexttask = task->firstChild();
    while ( rval.isEmpty() && nexttask )
    {
      rval = _hasTask( nexttask, taskname );
      nexttask = nexttask->nextSibling();
    }
  }
  return rval;
}

Task* karmPart::_hasUid( Task* task, const TQString &uid ) const
{
  Task *rval = NULL;

  //kdDebug(5970) << "MainWindow::_hasUid( " << task << ", " << uid << " )" << endl;

  if ( task->uid() == uid ) rval = task;
  else
  {
    Task* nexttask = task->firstChild();
    while ( !rval && nexttask )
    {
      rval = _hasUid( nexttask, uid );
      nexttask = nexttask->nextSibling();
    }
  }
  return rval;
}

TQString karmPart::starttimerfor( const TQString& taskname )
{
  TQString err="no such task";
  for (int i=0; i<_taskView->count(); i++)
  {
    if ((_taskView->item_at_index(i)->name()==taskname))
    {
      _taskView->startTimerFor( _taskView->item_at_index(i) );
      err="";
    }
  }
  return err;
}

TQString karmPart::stoptimerfor( const TQString& taskname )
{
  TQString err="no such task";
  for (int i=0; i<_taskView->count(); i++)
  {
    if ((_taskView->item_at_index(i)->name()==taskname))
    {
      _taskView->stopTimerFor( _taskView->item_at_index(i) );
      err="";
    }
  }
  return err;
}

TQString karmPart::exportcsvfile( TQString filename, TQString from, TQString to, int type, bool decimalMinutes, bool allTasks, TQString delimiter, TQString quote )
{
  ReportCriteria rc;
  rc.allTasks=allTasks;
  rc.decimalMinutes=decimalMinutes;
  rc.delimiter=delimiter;
  rc.from=TQDate::fromString( from );
  rc.quote=quote;
  rc.reportType=(ReportCriteria::REPORTTYPE) type;
  rc.to=TQDate::fromString( to );
  rc.url=filename;
  return _taskView->report( rc );
}

TQString karmPart::importplannerfile( TQString fileName )
{
  return _taskView->importPlanner(fileName);
}

void karmPart::startNewSession()
{
  _taskView->startNewSession();
  _taskView->save();
} 

#include <tqpopupmenu.h>
#include "karm_part.moc"
