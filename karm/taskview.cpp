#include <qdict.h>
#include <qfile.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qptrlist.h>
#include <qptrstack.h>
#include <qtextstream.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>            // i18n
#include <kmessagebox.h>
#include <kemailsettings.h>
#include <klineeditdlg.h>

#include "calendarlocal.h"
#include "event.h"
#include "todo.h"

#include "desktoptracker.h"
#include "edittaskdialog.h"
#include "idletimedetector.h"
#include "preferences.h"
#include "task.h"
#include "taskview.h"

#define T_LINESIZE 1023
#define HIDDEN_COLUMN -10

class DesktopTracker;

TaskView::TaskView( QWidget *parent, const char *name )
  : KListView( parent, name ),
    _calendar()
{
  _preferences = Preferences::instance();

  KEMailSettings settings;
  _calendar.setEmail( settings.getSetting( KEMailSettings::EmailAddress ) );
  _calendar.setOwner( settings.getSetting( KEMailSettings::RealName ) );

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
  // setColumnAlignment( 1, Qt::AlignRight );
  // setColumnAlignment( 2, Qt::AlignRight );
  // setColumnAlignment( 3, Qt::AlignRight );
  adaptColumns();
  setAllColumnsShowFocus( true );

  // set up the minuteTimer
  _minuteTimer = new QTimer(this);
  connect( _minuteTimer, SIGNAL( timeout() ), this, SLOT( minuteUpdate() ));
  _minuteTimer->start(1000 * secsPerMinute);

  // resize columns when config is changed
  connect( _preferences, SIGNAL( setupChanged() ), this,SLOT( adaptColumns() ));

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

void TaskView::load()
{
  if ( _preferences->useLegacyFileFormat() )
    loadFromFileFormat();
  else {
    loadFromKCalFormat();
  }
}

void TaskView::loadFromKCalFormat( const QString& file, int loadMask )
{
  bool loadOk = _calendar.load( file );
  if ( !loadOk ) {
    kdDebug() << "Failed to load the calendar!!!" << endl;
    return;
  }
  kdDebug() << "Loading karm calendar data from " << file << endl;

  if ( loadMask & TaskView::loadEvent ) {
    QPtrList<KCal::Event> eventList = _calendar.rawEvents();
    kdDebug() << "There are " << eventList.count()
              << " events in the calendar." << endl;
    buildAndPositionTasks( eventList );
  }

  if ( loadMask & TaskView::loadTodo ) {
    QPtrList<KCal::Todo> todoList = _calendar.rawTodos();
    kdDebug() << "There are " << todoList.count()
              << " todos in the calendar." << endl;
    buildAndPositionTasks( todoList );
  }

  adjustFromLegacyFileFormat();

  setSelected(firstChild(), true);
  setCurrentItem(firstChild());

  _desktopTracker->startTracking();
}

void TaskView::loadFromKCalFormat()
{
  loadFromKCalFormat( _preferences->loadFile(),
                      TaskView::loadEvent|TaskView::loadTodo );
}

void TaskView::loadFromKOrgTodos()
{
  loadFromKCalFormat( _preferences->activeCalendarFile(), TaskView::loadTodo );
}

void TaskView::loadFromKOrgEvents()
{
  loadFromKCalFormat( _preferences->activeCalendarFile(), TaskView::loadEvent );
}

void TaskView::buildAndPositionTasks( QPtrList<KCal::Event>& eventList )
{
  QDict< Task > uid_task_map;
  for ( QPtrListIterator<KCal::Event> iter ( eventList );
        iter.current();
        ++iter ) {
    buildTask( *iter, uid_task_map );
  }

  for ( QPtrListIterator<KCal::Event> iter ( eventList );
        iter.current();
        ++iter ) {
    positionTask( *iter, uid_task_map );
  }
}

void TaskView::buildAndPositionTasks( QPtrList<KCal::Todo>& todoList )
{
  QDict< Task > uid_task_map;
  for ( QPtrListIterator<KCal::Todo> iter ( todoList ); iter.current(); ++iter ) {
    buildTask( *iter, uid_task_map );
  }

  for ( QPtrListIterator<KCal::Todo> iter ( todoList ); iter.current(); ++iter ) {
    positionTask( *iter, uid_task_map );
  }
}

void TaskView::buildTask( KCal::Incidence* event, QDict<Task>& map )
{
  Task* task = new Task( event, this );
  map.insert( event->uid(), task );
  setRootIsDecorated(true);
  task->setOpen(true);
  _desktopTracker->registerForDesktops( task, task->getDesktops() );
}

void TaskView::positionTask( const KCal::Incidence* event,
                             const QDict<Task>& map )
{
  QString eventName = event->summary();
  if ( !event->relatedTo() ) {
    kdDebug() << eventName << ", has no relations" << endl;
    return;
  }

  Task* newParent = map.find( event->relatedToUid() );
  if ( !newParent ) {
    kdDebug() << "ERROR: Can't find the parent for " << eventName << endl;
    return;
  }

  Task* task = map.find( event->uid() );
  if ( !task ) {
    kdDebug() << "ERROR: Can't find the task " << eventName << endl;
    return;
  }

  QString parentName = newParent->name();
  QString taskName = task->name();
  kdDebug() << "Moving (" << taskName << ") under ("
            << parentName << ")" << endl;

  task->move( newParent);
}

void TaskView::loadFromFileFormat()
{
  QFile f(_preferences->loadFile());
  kdDebug() << "Loading karm data from " << f.name() << endl;

  if( !f.exists() )
    return;

  if( !f.open( IO_ReadOnly ) )
    return;

  QString line;

  QPtrStack<Task> stack;
  Task *task;

  QTextStream stream(&f);

  while( !stream.atEnd() ) {
    // lukas: this breaks for non-latin1 chars!!!
    // if ( file.readLine( line, T_LINESIZE ) == 0 )
    //   break;

    line = stream.readLine();
    kdDebug() << "DEBUG: line: " << line << "\n";

    if (line.isNull())
      break;

    long minutes;
    int level;
    QString name;
    DesktopList desktopList;
    if (!parseLine(line, &minutes, &name, &level, &desktopList))
      continue;

    unsigned int stackLevel = stack.count();
    for (unsigned int i = level; i<=stackLevel ; i++) {
      stack.pop();
    }

    if (level == 1) {
      kdDebug() << "DEBUG: toplevel task: " << name
                << " min: " << minutes << "\n";
      task = new Task(name, minutes, 0, desktopList, this);
    }
    else {
      Task *parent = stack.top();
      kdDebug() << "DEBUG:          task: " << name
                << " min: " << minutes << " parent" << parent->name() << "\n";
      task = new Task(name, minutes, 0, desktopList, parent);
      // Legacy File Format (!):
      parent->changeTimes(0, -minutes, false);
      setRootIsDecorated(true);
      parent->setOpen(true);
    }

    _desktopTracker->registerForDesktops(task, desktopList);

    stack.push(task);
  }

  // adjustFromLegacyFileFormat();

  f.close();

  setSelected(firstChild(), true);
  setCurrentItem(firstChild());

  _desktopTracker->startTracking();
}

void TaskView::adjustFromLegacyFileFormat()
{
  // This is code to read legacy file formats!
  //
  // Previously a task would save the cummulated (totalTime) in it's "time"
  // So if we are reading from an old File format we need to substract the
  // children's times from the parents' times
  if (    _preferences->fileFormat().isEmpty()
       || _preferences->fileFormat() == QString::fromLatin1("karm_kcal_1"))
    for ( Task* task = firstChild();
                task;
                task = task->nextSibling() ) {
       adjustFromLegacyFileFormat(task);
    }
}

void TaskView::adjustFromLegacyFileFormat(Task* task)
{
  // unless the parent is the listView
  if ( task->parent() )
    task->parent()->changeTimes( -task->sessionTime(), -task->time(), false);

  // traverse depth first -
  // as soon as we're in a leaf, we'll substract it's time from the parent
  // then, while descending back we'll do the same for each node untill
  // we reach the root
  for ( Task* subtask = task->firstChild();
              subtask;
              subtask = subtask->nextSibling() )
    adjustFromLegacyFileFormat(subtask);
}

bool TaskView::parseLine( QString line, long *time, QString *name, int *level,
                          DesktopList* desktopList)
{
  if (line.find('#') == 0) {
    // A comment line
    return false;
  }

  int index = line.find('\t');
  if (index == -1) {
    // This doesn't seem like a valid record
    return false;
  }

  QString levelStr = line.left(index);
  QString rest = line.remove(0,index+1);

  index = rest.find('\t');
  if (index == -1) {
    // This doesn't seem like a valid record
    return false;
  }

  QString timeStr = rest.left(index);
  rest = rest.remove(0,index+1);

  bool ok;

  index = rest.find('\t'); // check for optional desktops string
  if (index >= 0) {
    *name = rest.left(index);
    QString deskLine = rest.remove(0,index+1);

    // now transform the ds string (e.g. "3", or "1,4,5") into
    // an DesktopList
    QString ds;
    int d;
    int commaIdx = deskLine.find(',');
    while (commaIdx >= 0) {
      ds = deskLine.left(commaIdx);
      d = ds.toInt(&ok);
      if (!ok)
        return false;

      desktopList->push_back(d);
      deskLine.remove(0,commaIdx+1);
      commaIdx = deskLine.find(',');
    }

    d = deskLine.toInt(&ok);

    if (!ok)
      return false;

    desktopList->push_back(d);
  }
  else {
    *name = rest.remove(0,index+1);
  }

  *time = timeStr.toLong(&ok);

  if (!ok) {
    // the time field was not a number
    return false;
  }
  *level = levelStr.toInt(&ok);
  if (!ok) {
    // the time field was not a number
    return false;
  }

  return true;
}

void TaskView::save()
{
  saveToKCalFormat();
  // saveToFileFormat();
}

void TaskView::saveToKCalFormat()
{
  KCal::CalendarLocal cal;
  KEMailSettings settings;
  cal.setEmail( settings.getSetting( KEMailSettings::EmailAddress ) );
  cal.setOwner( settings.getSetting( KEMailSettings::RealName ) );

  QPtrStack< KCal::Event > parents;

  for ( Task* child = firstChild();
              child;
              child = child->nextSibling() ) {
    writeTaskToCalendar( cal, child, 1, parents );
  }

  cal.save( _preferences->saveFile() );
  kdDebug() << "Saved data to calendar file " << _preferences->saveFile() << endl;
}

void TaskView::saveToFileFormat()
{
  QFile f(_preferences->saveFile());

  if ( !f.open( IO_WriteOnly | IO_Truncate ) ) {
    QString msg = i18n( "There was an error trying to save your data file.\n"
                       "Time accumulated during this session will not be saved!\n");
    KMessageBox::error(0, msg );
    return;
  }
  const char * comment = "# TaskView save data\n";

  f.writeBlock(comment, strlen(comment));  //comment
  f.flush();

  QTextStream stream(&f);
  for (Task* child = firstChild();
             child;
             child = child->nextSibling())
    writeTaskToFile(&stream, child, 1);

  f.close();
  kdDebug() << "Saved data to file " << f.name() << endl;
}

void TaskView::writeTaskToCalendar( KCal::CalendarLocal& cal, Task* task,
                                    int level,
                                    QPtrStack< KCal::Event >& parents )
{
  KCal::Event* event = task->asEvent( level );
  if ( !parents.isEmpty() ) {
    event->setRelatedTo( parents.top() );
  }

  parents.push( event );

  cal.addEvent( event );

  for ( Task* nextTask = task->firstChild();
        nextTask;
        nextTask = nextTask->nextSibling() ) {
    writeTaskToCalendar( cal, nextTask, level+1, parents );
  }

  parents.pop();
}

void TaskView::writeTaskToFile( QTextStream *strm, Task *task,
                                int level)
{
  //lukas: correct version for non-latin1 users
  QString _line = QString::fromLatin1("%1\t%2\t%3").arg(level).
          arg(task->time()).arg(task->name());

  DesktopList d = task->getDesktops();
  int dsize = d.size();
  if (dsize>0) {
    _line += '\t';
    for (int i=0; i<dsize-1; i++) {
      _line += QString::number(d[i]);
      _line += ',';
    }
    _line += QString::number(d[dsize-1]);
  }
  *strm << _line << "\n";

  for ( Task* child= task->firstChild();
              child;
              child=child->nextSibling()) {
    writeTaskToFile(strm, child, level+1);
  }
}

void TaskView::startCurrentTimer()
{
  startTimerFor( currentItem() );
}

void TaskView::startTimerFor(Task* task)
{
  if (task != 0 && activeTasks.findRef(task) == -1) {
    _idleTimeDetector->startIdleDetection();
    task->setRunning(true);
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
    activeTasks.at(i)->setRunning(false);
  }
  _idleTimeDetector->stopIdleDetection();
  activeTasks.clear();
  emit updateButtons();
  emit timersInactive();
  emit tasksChanged( activeTasks);
}

void TaskView::startNewSession()
{
  QListViewItemIterator item( firstChild());
  for ( ; item.current(); ++item ) {
    Task * task = (Task *) item.current();
    task->startNewSession();
  }
}

void TaskView::resetTimeForAllTasks()
{
  QListViewItemIterator item( firstChild());
  for ( ; item.current(); ++item ) {
    Task * task = (Task *) item.current();
    task->resetTimes();
  }
}

void TaskView::stopTimerFor(Task* task)
{
  if (task != 0 && activeTasks.findRef(task) != -1) {
    activeTasks.removeRef(task);
    task->setRunning(false);
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
  stopTimerFor( currentItem());
}


void TaskView::changeTimer(QListViewItem *)
{
  Task *task = currentItem();
  if (task != 0 && activeTasks.findRef(task) == -1) {
    // Stop all the other timers.
    for (unsigned int i=0; i<activeTasks.count();i++) {
      (activeTasks.at(i))->setRunning(false);
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
    activeTasks.at(i)->changeTime(minutes, do_logging);
}

void TaskView::newTask()
{
  newTask(i18n("New Task"), 0);
}

void TaskView::newTask(QString caption, Task *parent)
{
  EditTaskDialog *dialog = new EditTaskDialog(caption, false);
  int result = dialog->exec();

  if (result == QDialog::Accepted) {
    QString taskName = i18n("Unnamed Task");
    if (!dialog->taskName().isEmpty()) {
      taskName = dialog->taskName();
    }

    long total, totalDiff, session, sessionDiff;
    total = totalDiff = session = sessionDiff = 0;
    DesktopList desktopList;
    dialog->status( &total, &totalDiff, &session, &sessionDiff, &desktopList);
    Task *task;

    // If all available desktops are checked, disable auto tracking,
    // since it makes no sense to track for every desktop.
    if (desktopList.size() == (unsigned int)_desktopTracker->desktopCount())
      desktopList.clear();

    if (parent == 0)
      task = new Task(taskName, total, session, desktopList, this);
    else
      task = new Task(taskName, total, session, desktopList, parent);

    _desktopTracker->registerForDesktops( task, desktopList );

    setCurrentItem( task );
    setSelected( task, true );
  }
  delete dialog;
}

void TaskView::newSubTask()
{
  Task* task = currentItem();
  if(!task)
    return;
  newTask(i18n("New Sub Task"), task);
  task->setOpen(true);
  setRootIsDecorated(true);
}

void TaskView::editTask()
{
  Task *task = currentItem();
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
    task->setName(taskName);

    // update session time as well if the time was changed
    long total, session, totalDiff, sessionDiff;
    total = totalDiff = session = sessionDiff = 0;
    DesktopList desktopList;
    dialog->status( &total, &totalDiff, &session, &sessionDiff, &desktopList);

    if( totalDiff != 0 || sessionDiff != 0)
      task->changeTimes( sessionDiff ,totalDiff, true );

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

void TaskView::addCommentToTask()
{
  Task *task = currentItem();
  if (!task)
    return;

  bool ok;
  QString comment = KLineEditDlg::getText(
                       i18n("Log comment for task %1").arg(task->name()),
                       QString(), &ok, this);
  if ( ok )
    task->addComment( comment );
}

void TaskView::deleteTask()
{
  Task *task = currentItem();
  if (task == 0) {
    KMessageBox::information(0,i18n("No task selected"));
    return;
  }

  int response = KMessageBox::Yes;
  if ( _preferences->promptDelete() ) {
    if (task->childCount() == 0) {
      response = KMessageBox::questionYesNo( 0,
                  i18n( "Are you sure you want to delete "
                        "the task named\n\"%1\"").arg(task->name()),
                  i18n( "Deleting Task"));
    }
    else {
      response = KMessageBox::questionYesNo( 0,
                  i18n( "Are you sure you want to delete the task named"
                        "\n\"%1\"\n" "NOTE: all its subtasks will also "
                        "be deleted!").arg(task->name()),
                  i18n( "Deleting Task"));
    }
  }

  if (response == KMessageBox::Yes) {

    task->remove( activeTasks);
    
    // remove root decoration if there is no more children.
    bool anyChilds = false;
    for(Task* child = firstChild();
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
    if (!_autoSaveTimer->isActive()) {
      _autoSaveTimer->start(_preferences->autoSavePeriod()*1000*secsPerMinute);
    }
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
#include "taskview.moc"
