#include <qfile.h>
#include <qclipboard.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qptrlist.h>
#include <qptrstack.h>
#include <qtextstream.h>
#include <qlistview.h>
#include <qtimer.h>

#include "kapplication.h"       // kapp
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>            // i18n
#include <kfiledialog.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>

#include "desktoptracker.h"
#include "edittaskdialog.h"
#include "idletimedetector.h"
#include "preferences.h"
#include "task.h"
#include "taskview.h"
#include "timekard.h"
#include "karmstorage.h"
#include "printdialog.h"

#define T_LINESIZE 1023
#define HIDDEN_COLUMN -10

class DesktopTracker;

TaskView::TaskView(QWidget *parent, const char *name):KListView(parent,name)
{
  _preferences = Preferences::instance();
  _storage = KarmStorage::instance();

  connect(this, SIGNAL( doubleClicked( QListViewItem * )),
          this, SLOT( changeTimer( QListViewItem * )));

  // setup default values
  previousColumnWidths[0] = previousColumnWidths[1]
  = previousColumnWidths[2] = previousColumnWidths[3] = HIDDEN_COLUMN;

  addColumn( i18n("Task Name") );
  addColumn( i18n("Session Time") );
  addColumn( i18n("Time") );
  addColumn( i18n("Total Session Time") );
  addColumn( i18n("Total Time") );
  setColumnAlignment( 1, Qt::AlignRight );
  setColumnAlignment( 2, Qt::AlignRight );
  setColumnAlignment( 3, Qt::AlignRight );
  setColumnAlignment( 4, Qt::AlignRight );
  adaptColumns();
  setAllColumnsShowFocus( true );

  // set up the minuteTimer
  _minuteTimer = new QTimer(this);
  connect( _minuteTimer, SIGNAL( timeout() ), this, SLOT( minuteUpdate() ));
  _minuteTimer->start(1000 * secsPerMinute);

  // React when user changes iCalFile
  connect(_preferences, SIGNAL(iCalFile(QString)),
      this, SLOT(iCalFileChanged(QString)));

  // resize columns when config is changed
  connect(_preferences, SIGNAL( setupChanged() ), this,SLOT( adaptColumns() ));

  _minuteTimer->start(1000 * secsPerMinute);

  // Set up the idle detection.
  _idleTimeDetector = new IdleTimeDetector( _preferences->idlenessTimeout() );
  connect( _idleTimeDetector, SIGNAL( extractTime(int) ),
           this, SLOT( extractTime(int) ));
  connect( _idleTimeDetector, SIGNAL( stopAllTimers() ),
           this, SLOT( stopAllTimers() ));
  connect( _preferences, SIGNAL( idlenessTimeout(int) ),
           _idleTimeDetector, SLOT( setMaxIdle(int) ));
  connect( _preferences, SIGNAL( detectIdleness(bool) ),
           _idleTimeDetector, SLOT( toggleOverAllIdleDetection(bool) ));
  if (!_idleTimeDetector->isIdleDetectionPossible())
    _preferences->disableIdleDetection();

  // Setup auto save timer
  _autoSaveTimer = new QTimer(this);
  connect( _preferences, SIGNAL( autoSave(bool) ),
           this, SLOT( autoSaveChanged(bool) ));
  connect( _preferences, SIGNAL( autoSavePeriod(int) ),
           this, SLOT( autoSavePeriodChanged(int) ));
  connect( _autoSaveTimer, SIGNAL( timeout() ), this, SLOT( save() ));

  // Setup manual save timer (to save changes a little while after they happen)
  _manualSaveTimer = new QTimer(this);
  connect( _manualSaveTimer, SIGNAL( timeout() ), this, SLOT( save() ));

  // Connect desktop tracker events to task starting/stopping
  _desktopTracker = new DesktopTracker();
  connect( _desktopTracker, SIGNAL( reachedtActiveDesktop( Task* ) ),
           this, SLOT( startTimerFor(Task*) ));
  connect( _desktopTracker, SIGNAL( leftActiveDesktop( Task* ) ),
           this, SLOT( stopTimerFor(Task*) ));
}

TaskView::~TaskView()
{
  _preferences->save();
}

Task* TaskView::first_child() const
{
  return static_cast<Task*>(firstChild());
}

Task* TaskView::current_item() const
{
  return static_cast<Task*>(currentItem());
}

Task* TaskView::item_at_index(int i)
{
  return static_cast<Task*>(itemAtIndex(i));
}

void TaskView::load()
{

  QString err = _storage->load(this, _preferences);

  if (!err.isEmpty())
  {
    KMessageBox::error(this, err);
    return;
  }

  // TODO: If empty, ask if user wants to import tasks from another file
  //if (_storage.isEmpty())
  //{
  //}

  // Register tasks with desktop tracker
  int task_idx = 0;
  for (Task* task = item_at_index(task_idx);
      task;
      task = item_at_index(++task_idx))
  {
    _desktopTracker->registerForDesktops( task, task->getDesktops() );
  }

  setSelected(first_child(), true);
  setCurrentItem(first_child());
  _desktopTracker->startTracking();
}

void TaskView::loadFromFlatFile()
{
  kdDebug(5970) << "TaskView::loadFromFlatFile()" << endl;

  //KFileDialog::getSaveFileName("icalout.ics",i18n("*.ics|ICalendars"),this);

  QString fileName(KFileDialog::getOpenFileName(QString::null, QString::null,
        0));
  if (!fileName.isEmpty()) {
    QString err = _storage->loadFromFlatFile(this, fileName);
    if (!err.isEmpty())
    {
      KMessageBox::error(this, err);
      return;
    }

    // Register tasks with desktop tracker
    int task_idx = 0;
    Task* task = item_at_index(task_idx++);
    while (task)
    {
      // item_at_index returns 0 where no more items.
      _desktopTracker->registerForDesktops( task, task->getDesktops() );
      task = item_at_index(task_idx++);
    }

    setSelected(first_child(), true);
    setCurrentItem(first_child());

    _desktopTracker->startTracking();
  }
}

void TaskView::scheduleSave()
{
    _manualSaveTimer->start( 10, true /*single-shot*/ );
}

void TaskView::save()
{
    // DF: this code created a new event for the running task(s),
    // at every call (very frequent with autosave) !!!
    // -> if one wants autosave to save the current event, then
    // Task needs to store the "current event" and we need to update
    // it before calling save.
#if 0
  // Stop then start all timers so history entries are written.  This is
  // inefficient if more than one task running, but it is correct.  It is
  // inefficient because the iCalendar file is saved every time a task's
  // setRunning(false, ...) is called.  For a big ics file, this could be a
  // drag.  However, it does ensure that the data will be consistent.  And
  // if the most common use case is that one task is running most of the time,
  // it won't make any difference.
  for (unsigned int i = 0; i < activeTasks.count(); i++)
  {
    activeTasks.at(i)->setRunning(false, _storage);
    activeTasks.at(i)->setRunning(true, _storage);
  }

  // If there was an active task, the iCal file has already been saved.
  if (activeTasks.count() == 0)
#endif
  {
    _storage->save(this);
  }
}

void TaskView::startCurrentTimer()
{
  startTimerFor( current_item() );
}

long TaskView::count()
{
  long n = 0;
  for (Task* t = item_at_index(n); t; t=item_at_index(++n));
  return n;
}

void TaskView::startTimerFor(Task* task)
{
  if (task != 0 && activeTasks.findRef(task) == -1) {
    _idleTimeDetector->startIdleDetection();
    task->setRunning(true, _storage);
    activeTasks.append(task);
    emit updateButtons();
    if ( activeTasks.count() == 1 )
        emit timersActive();

    emit tasksChanged( activeTasks);
  }
}

void TaskView::stopAllTimers()
{
  for (unsigned int i=0; i<activeTasks.count();i++) {
    activeTasks.at(i)->setRunning(false, _storage);
  }
  _idleTimeDetector->stopIdleDetection();
  activeTasks.clear();
  emit updateButtons();
  emit timersInactive();
  emit tasksChanged( activeTasks);
}

void TaskView::startNewSession()
{
  QListViewItemIterator item( first_child());
  for ( ; item.current(); ++item ) {
    Task * task = (Task *) item.current();
    task->startNewSession();
  }
}

void TaskView::resetTimeForAllTasks()
{
  QListViewItemIterator item( first_child());
  for ( ; item.current(); ++item ) {
    Task * task = (Task *) item.current();
    task->resetTimes();
  }
}

void TaskView::stopTimerFor(Task* task)
{
  if (task != 0 && activeTasks.findRef(task) != -1) {
    activeTasks.removeRef(task);
    task->setRunning(false, _storage);
    if (activeTasks.count()== 0) {
      _idleTimeDetector->stopIdleDetection();
      emit timersInactive();
    }
    emit updateButtons();
  }
  emit tasksChanged( activeTasks);
}

void TaskView::stopCurrentTimer()
{
  stopTimerFor( current_item());
}


void TaskView::changeTimer(QListViewItem *)
{
  Task *task = current_item();
  if (task != 0 && activeTasks.findRef(task) == -1) {
    // Stop all the other timers.
    for (unsigned int i=0; i<activeTasks.count();i++) {
      (activeTasks.at(i))->setRunning(false, _storage);
    }
    activeTasks.clear();

    // Start the new timer.
    startCurrentTimer();
  }
  else {
    stopCurrentTimer();
  }
}

void TaskView::minuteUpdate()
{
  addTimeToActiveTasks(1, false);
}

void TaskView::addTimeToActiveTasks(int minutes, bool do_logging)
{
  for(unsigned int i=0; i<activeTasks.count();i++)
    activeTasks.at(i)->changeTime(minutes, do_logging, _storage);
}

void TaskView::newTask()
{
  newTask(i18n("New Task"), 0);
}

void TaskView::newTask(QString caption, Task *parent)
{
  EditTaskDialog *dialog = new EditTaskDialog(caption, false);
  long total, totalDiff, session, sessionDiff;
  DesktopList desktopList;
  Task *task;

  int result = dialog->exec();
  if (result == QDialog::Accepted) {
    QString taskName = i18n("Unnamed Task");
    if (!dialog->taskName().isEmpty()) {
      taskName = dialog->taskName();
    }

    total = totalDiff = session = sessionDiff = 0;
    dialog->status( &total, &totalDiff, &session, &sessionDiff, &desktopList);

    // If all available desktops are checked, disable auto tracking,
    // since it makes no sense to track for every desktop.
    if (desktopList.size() == (unsigned int)_desktopTracker->desktopCount())
      desktopList.clear();

    if (parent == 0)
    {
      task = new Task(taskName, total, session, desktopList, this);
      task->setUid(_storage->addTask(task, 0));
    }
    else
    {
      task = new Task(taskName, total, session, desktopList, parent);
      task->setUid(_storage->addTask(task, parent));
    }

    if (!task->uid().isNull())
    {
      _desktopTracker->registerForDesktops( task, desktopList );

      setCurrentItem( task );
      setSelected( task, true );

      save();
    }
    else
    {
      delete task;
      KMessageBox::error(0,i18n(
            "Error storing new task. Your changes were not saved."));
    }
  }

  delete dialog;
}

void TaskView::newSubTask()
{
  Task* task = current_item();
  if(!task)
    return;
  newTask(i18n("New Sub Task"), task);
  task->setOpen(true);
  setRootIsDecorated(true);
}

void TaskView::editTask()
{
  Task *task = current_item();
  if (!task)
    return;

  DesktopList desktopList = task->getDesktops();
  EditTaskDialog *dialog = new EditTaskDialog(i18n("Edit Task"), true, &desktopList);
  dialog->setTask( task->name(),
                   task->time(),
                   task->sessionTime() );
  int result = dialog->exec();
  if (result == QDialog::Accepted) {
    QString taskName = i18n("Unnamed Task");
    if (!dialog->taskName().isEmpty()) {
      taskName = dialog->taskName();
    }
    // setName only does something if the new name is different
    task->setName(taskName, _storage);

    // update session time as well if the time was changed
    long total, session, totalDiff, sessionDiff;
    total = totalDiff = session = sessionDiff = 0;
    DesktopList desktopList;
    dialog->status( &total, &totalDiff, &session, &sessionDiff, &desktopList);

    if( totalDiff != 0 || sessionDiff != 0)
      task->changeTimes( sessionDiff ,totalDiff, true, _storage );

    // If all available desktops are checked, disable auto tracking,
    // since it makes no sense to track for every desktop.
    if (desktopList.size() == (unsigned int)_desktopTracker->desktopCount())
      desktopList.clear();

    task->setDesktopList(desktopList);

    _desktopTracker->registerForDesktops( task, desktopList );

    emit updateButtons();
  }
  delete dialog;
}

//void TaskView::addCommentToTask()
//{
//  Task *task = current_item();
//  if (!task)
//    return;

//  bool ok;
//  QString comment = KLineEditDlg::getText(i18n("Comment"),
//                       i18n("Log comment for task '%1':").arg(task->name()),
//                       QString(), &ok, this);
//  if ( ok )
//    task->addComment( comment, _storage );
//}


void TaskView::deleteTask(bool markingascomplete)
{
  Task *task = current_item();
  if (task == 0) {
    KMessageBox::information(0,i18n("No task selected."));
    return;
  }

  int response = KMessageBox::Yes;
  if (!markingascomplete && _preferences->promptDelete()) {
    if (task->childCount() == 0) {
      response = KMessageBox::warningYesNo( 0,
          i18n( "Are you sure you want to delete "
          "the task named\n\"%1\" and its entire history?")
          .arg(task->name()),
          i18n( "Deleting Task"));
    }
    else {
      response = KMessageBox::warningYesNo( 0,
          i18n( "Are you sure you want to delete the task named"
          "\n\"%1\" and its entire history?\n"
          "NOTE: all its subtasks and their history will also "
          "be deleted!").arg(task->name()),
          i18n( "Deleting Task"));
    }
  }

  if (response == KMessageBox::Yes)
  {
    if (markingascomplete)
    {
      task->setPercentComplete(100, _storage);
      save();

      // Have to remove after saving, as the save routine only affects tasks
      // that are in the view.  Otherwise, the new percent complete does not
      // get saved.   (No longer remove when marked as complete.)
      //task->removeFromView();

    }
    else
    {
      //kdDebug(5970) << "TaskView::deleteTask - 1" << endl;
      task->remove(activeTasks, _storage);
      //kdDebug(5970) << "TaskView::deleteTask - 2" << endl;
      task->removeFromView();
      //kdDebug(5970) << "TaskView::deleteTask - 3" << endl;
      save();
      //kdDebug(5970) << "TaskView::deleteTask - 4" << endl;
    }

    // remove root decoration if there is no more children.
    bool anyChilds = false;
    for(Task* child = first_child();
              child;
              child = child->nextSibling()) {
      if (child->childCount() != 0) {
        anyChilds = true;
        break;
      }
    }
    if (!anyChilds) {
      setRootIsDecorated(false);
    }

    // Stop idle detection if no more counters are running
    if (activeTasks.count() == 0) {
      _idleTimeDetector->stopIdleDetection();
      emit timersInactive();
    }

    emit tasksChanged( activeTasks );
  }
}

void TaskView::extractTime(int minutes)
{
  addTimeToActiveTasks(-minutes, true);
}

void TaskView::autoSaveChanged(bool on)
{
  if (on) {
    _autoSaveTimer->start(_preferences->autoSavePeriod()*1000*secsPerMinute);
  }
  else {
    if (_autoSaveTimer->isActive()) {
      _autoSaveTimer->stop();
    }
  }
}

void TaskView::autoSavePeriodChanged(int /*minutes*/)
{
  autoSaveChanged(_preferences->autoSave());
}

void TaskView::adaptColumns()
{
  // to hide a column X we set it's width to 0
  // at that moment we'll remember the original column within
  // previousColumnWidths[X]
  //
  // When unhiding a previously hidden column
  // (previousColumnWidths[X] != HIDDEN_COLUMN !)
  // we restore it's width from the saved value and set
  // previousColumnWidths[X] to HIDDEN_COLUMN

  for( int x=1; x <= 4; x++) {
    // the column was invisible before and were switching it on now
    if(   _preferences->displayColumn(x-1)
       && previousColumnWidths[x-1] != HIDDEN_COLUMN )
    {
      setColumnWidth( x, previousColumnWidths[x-1] );
      previousColumnWidths[x-1] = HIDDEN_COLUMN;
      setColumnWidthMode( x, QListView::Maximum );
    }
    // the column was visible before and were switching it off now
    else
    if( ! _preferences->displayColumn(x-1)
       && previousColumnWidths[x-1] == HIDDEN_COLUMN )
    {
      setColumnWidthMode( x, QListView::Manual ); // we don't want update()
                                                  // to resize/unhide the col
      previousColumnWidths[x-1] = columnWidth( x );
      setColumnWidth( x, 0 );
    }
  }
}

void TaskView::deletingTask(Task* deletedTask)
{
  DesktopList desktopList;

  _desktopTracker->registerForDesktops( deletedTask, desktopList );
  activeTasks.removeRef( deletedTask );

  emit tasksChanged( activeTasks);
}

void TaskView::iCalFileChanged(QString file)
{
  kdDebug(5970) << "TaskView:iCalFileChanged: " << file << endl;
  load();
}

QValueList<HistoryEvent> TaskView::getHistory(const QDate& from,
    const QDate& to) const
{
  return _storage->getHistory(from, to);
}

void TaskView::markTaskAsComplete()
{
  if (current_item())
    kdDebug(5970) << "TaskView::markTaskAsComplete: "
      << current_item()->uid() << endl;
  else
    kdDebug(5970) << "TaskView::markTaskAsComplete: null current_item()" << endl;

  bool markingascomplete = true;
  deleteTask(markingascomplete);
}

void TaskView::clipTotals()
{
  TimeKard *t = new TimeKard();
  if (current_item() && current_item()->isRoot())
  {
    int response = KMessageBox::questionYesNo( 0,
        i18n("Copy totals for just this task and its subtasks?"
          "  (Click No to copy totals for all tasks.)"));
    if (response == KMessageBox::Yes)
    {
      KApplication::clipboard()->setText(t->totalsAsText(this));
    }
    else
    {
      KApplication::clipboard()->setText(t->totalsAsText(this, false));
    }
  }
  else
  {
    KApplication::clipboard()->setText(t->totalsAsText(this));
  }
}

void TaskView::clipHistory()
{

  PrintDialog *dialog = new PrintDialog();
  if (dialog->exec()== QDialog::Accepted)
  {
    TimeKard *t = new TimeKard();
    KApplication::clipboard()->
      setText(t->historyAsText(this, dialog->from(), dialog->to()));
  }
}

#include "taskview.moc"
