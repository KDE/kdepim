#include <numeric>
#include <functional>
#include <algorithm>
#include <qptrstack.h>
#include <qptrlist.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <qlistbox.h>
#include <qlayout.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qtimer.h>
#include <qdict.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kmessagebox.h>
#include <kemailsettings.h>
#include <kstandarddirs.h>

#include "event.h"
#include "calendarlocal.h"

#include "task.h"
#include "taskview.h"
#include "addtaskdialog.h"
#include "idletimedetector.h"
#include "preferences.h"
#include "kdebug.h"
#include "karmutility.h"

#define T_LINESIZE 1023

TaskView::TaskView( QWidget *parent, const char *name )
  : KListView( parent, name ), _calendar()
{
  _preferences = Preferences::instance();

  KEMailSettings settings;
  _calendar.setEmail( settings.getSetting( KEMailSettings::EmailAddress ) );
  _calendar.setOwner( settings.getSetting( KEMailSettings::RealName ) );

  connect(this, SIGNAL( doubleClicked( QListViewItem * )),
          this, SLOT( changeTimer( QListViewItem * )));

  addColumn( i18n("Task Name") );
  addColumn( i18n("Session Time") );
  addColumn( i18n("Total Time") );
  setAllColumnsShowFocus( true );

  // set up the minuteTimer
  _minuteTimer = new QTimer(this);
  connect( _minuteTimer, SIGNAL( timeout() ), this, SLOT( minuteUpdate() ));
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

  connect( &kWinModule, SIGNAL( currentDesktopChanged(int) ),
           this, SLOT( handleDesktopChange(int) ));

  desktopCount = kWinModule.numberOfDesktops();
  lastDesktop = kWinModule.currentDesktop()-1;
  // currentDesktop will return 0 if no window manager is started
  if( lastDesktop < 0 ) lastDesktop = 0;
}

TaskView::~TaskView()
{
  _preferences->save();
}

void TaskView::handleDesktopChange(int desktop)
{
  desktop--; // desktopTracker starts with 0 for desktop 1
  // start all tasks setup for running on desktop
  TaskVector::iterator it;

  // stop trackers for lastDesktop
  TaskVector tv = desktopTracker[lastDesktop];
  for (it = tv.begin(); it != tv.end(); it++) {
    stopTimerFor(*it);
  }

  // start trackers for desktop
  tv = desktopTracker[desktop];
  for (it = tv.begin(); it != tv.end(); it++) {
    startTimerFor(*it);
  }
  lastDesktop = desktop;

  emit updateButtons();
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

  // Calculate totals for the statusbar (from toplevel tasks only)
  for ( QListViewItem* child = firstChild();
        child;
        child = child->nextSibling() )
    emit sessionTimeChanged( 0, static_cast<Task *>(child)->totalTime() );

  setSelected(firstChild(), true);
  setCurrentItem(firstChild());

  applyTrackers();
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
  updateTrackers( task, task->getDesktops() );
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
  QString parentName = newParent->name();

  Task* task = map.find( event->uid() );
  if ( !task ) {
    kdDebug() << "ERROR: Can't find the task " << eventName << endl;
    return;
  }
  QString taskName = task->name();

  kdDebug() << "Moving (" << taskName << ") under ("
            << parentName << ")" << endl;

  takeItem( task );
  newParent->insertItem( task );
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

    if (line.isNull())
      break;

    long minutes;
    int level;
    QString name;
    DesktopListType desktops;
    if (!parseLine(line, &minutes, &name, &level, &desktops))
      continue;

    unsigned int stackLevel = stack.count();
    for (unsigned int i = level; i<=stackLevel ; i++) {
      stack.pop();
    }

    if (level == 1) {
      task = new Task(name, minutes, 0, desktops, this);
      emit( sessionTimeChanged( 0, minutes ) );
    }
    else {
      Task *parent = stack.top();
      task = new Task(name, minutes, 0, desktops, parent);
      //emit( sessionTimeChanged( 0, minutes ) );
      setRootIsDecorated(true);
      parent->setOpen(true);
    }

    // update desktop trackers
    updateTrackers(task, desktops);

    stack.push(task);
  }
  f.close();

  setSelected(firstChild(), true);
  setCurrentItem(firstChild());

  applyTrackers();
}

void TaskView::applyTrackers()
{
  int currentDesktop = kWinModule.currentDesktop() -1;
  // currentDesktop will return 0 if no window manager is started
  if ( currentDesktop < 0 ) currentDesktop = 0;

  TaskVector &tv = desktopTracker[ currentDesktop ];
  TaskVector::iterator tit = tv.begin();
  while(tit!=tv.end()) {
    startTimerFor(*tit);
    tit++;
  }
}

void TaskView::updateTrackers(Task *task, DesktopListType desktopList)
{
  // if no desktop is marked, disable auto tracking for this task
  if (desktopList.size()==0) {
    for (int i=0; i<16; i++) {
      TaskVector *v = &(desktopTracker[i]);
      TaskVector::iterator tit = std::find(v->begin(), v->end(), task);
      if (tit != v->end())
        desktopTracker[i].erase(tit);
    }

    return;
  }

  // If desktop contains entries then configure desktopTracker
  // If a desktop was disabled, it will not be stopped automatically.
  // If enabled: Start it now.
  if (desktopList.size()>0) {
    for (int i=0; i<16; i++) {
      TaskVector& v = desktopTracker[i];
      TaskVector::iterator tit = std::find(v.begin(), v.end(), task);
      // Is desktop i in the desktop list?
      if ( std::find( desktopList.begin(), desktopList.end(), i)
           != desktopList.end()) {
        if (tit == v.end())  // not yet in start vector
          v.push_back(task); // track in desk i
      }
      else { // delete it
        if (tit != v.end())  // not in start vector any more
          v.erase(tit); // so we delete it from desktopTracker
      }
    }
    // printTrackers();
    applyTrackers();
  }
}

void TaskView::printTrackers() {
  TaskVector::iterator it;
  for (int i=0; i<16; i++) {
    TaskVector& start = desktopTracker[i];
    it = start.begin();
    while (it != start.end()) {
      it++;
    }
  }
}

bool TaskView::parseLine( QString line, long *time, QString *name, int *level,
                          DesktopListType* desktops)
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
    // an DesktopListType
    QString ds;
    int d;
    int commaIdx = deskLine.find(',');
    while (commaIdx >= 0) {
      ds = deskLine.left(commaIdx);
      d = ds.toInt(&ok);
      if (!ok)
        return false;

      desktops->push_back(d);
      deskLine.remove(0,commaIdx+1);
      commaIdx = deskLine.find(',');
    }

    d = deskLine.toInt(&ok);

    if (!ok)
      return false;

    desktops->push_back(d);
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

  for ( QListViewItem* child = firstChild(); child; child = child->nextSibling() ) {
    writeTaskToCalendar( cal, static_cast<Task*>( child ), 1, parents );
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
  for (QListViewItem *child =firstChild(); child; child = child->nextSibling())
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

  for ( QListViewItem* nextTask = task->firstChild();
        nextTask;
        nextTask = nextTask->nextSibling() ) {
    writeTaskToCalendar( cal, static_cast<Task*>( nextTask ), level+1, parents );
  }

  parents.pop();
}

void TaskView::writeTaskToFile(QTextStream *strm, QListViewItem *item, int level)
{
  Task * task = (Task *) item;
  //lukas: correct version for non-latin1 users
  QString _line = QString::fromLatin1("%1\t%2\t%3").arg(level).
          arg(task->totalTime()).arg(task->name());

  DesktopListType d = task->getDesktops();
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

  QListViewItem * child;
  for (child=item->firstChild(); child; child=child->nextSibling()) {
    writeTaskToFile(strm, child, level+1);
  }
}

void TaskView::startCurrentTimer()
{
  startTimerFor((Task *) currentItem());
}

void TaskView::startTimerFor(Task* item)
{
  if (item != 0 && activeTasks.findRef(item) == -1) {
    _idleTimeDetector->startIdleDetection();
    item->setRunning(true);
    activeTasks.append(item);
    emit updateButtons();
    if ( activeTasks.count() == 1 )
        emit timerActive();

    emit tasksChanged( activeTasks);
  }
}

void TaskView::stopAllTimers()
{
  for(unsigned int i=0; i<activeTasks.count();i++) {
    activeTasks.at(i)->setRunning(false);
  }
  _idleTimeDetector->stopIdleDetection();
  activeTasks.clear();
  emit updateButtons();
  emit timerInactive();
  emit tasksChanged( activeTasks);
}

void TaskView::resetSessionTimeForAllTasks()
{
  QListViewItemIterator item( firstChild());
  for ( ; item.current(); ++item ) {
    Task * task = (Task *) item.current();
    long sessionTime = task->sessionTime();
    task->setSessionTime(0);
    if ( !item.current()->parent() )
      sessionTimeChanged( -sessionTime, 0 );
  }
}

void TaskView::resetTimeForAllTasks()
{
  QListViewItemIterator item( firstChild());
  for ( ; item.current(); ++item ) {
    Task * task = (Task *) item.current();
    long sessionTime = task->sessionTime();
    long totalTime   = task->totalTime();
    task->setSessionTime(0);
    task->setTotalTime(0);
    if ( !item.current()->parent() )
      sessionTimeChanged( -sessionTime, -totalTime );
  }
}

void TaskView::stopTimerFor(Task* item)
{
  if (item != 0 && activeTasks.findRef(item) != -1) {
    activeTasks.removeRef(item);
    item->setRunning(false);
    if (activeTasks.count()== 0) {
      _idleTimeDetector->stopIdleDetection();
      emit timerInactive();
    }
    emit updateButtons();
  }
    emit tasksChanged( activeTasks);
}

void TaskView::stopCurrentTimer()
{
  stopTimerFor((Task *) currentItem());
}


void TaskView::changeTimer(QListViewItem *)
{
  Task *item = ((Task *) currentItem());
  if (item != 0 && activeTasks.findRef(item) == -1) {
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
  addTimeToActiveTasks(1);
}

void TaskView::addTimeToActiveTasks(int minutes)
{
  for(unsigned int i=0; i<activeTasks.count();i++) {
    Task *task = activeTasks.at(i);
    QListViewItem *item = task;
    while (item) {
      ((Task *) item)->incrementTime(minutes);
      item = item->parent();
    }
    emit( sessionTimeChanged( minutes, minutes ) );
  }
}

void TaskView::newTask()
{
  newTask(i18n("New Task"), 0);
}

void TaskView::newTask(QString caption, QListViewItem *parent)
{
  AddTaskDialog *dialog = new AddTaskDialog(caption, false);
  int result = dialog->exec();

  if (result == QDialog::Accepted) {
    QString taskName = i18n("Unnamed Task");
    if (!dialog->taskName().isEmpty()) {
      taskName = dialog->taskName();
    }

    long total, totalDiff, session, sessionDiff;
    total = totalDiff = session = sessionDiff = 0;
    DesktopListType desktopList;
    dialog->status( &total, &totalDiff, &session, &sessionDiff, &desktopList);
    Task *task;
    if (parent == 0)
      task = new Task(taskName, total, session, desktopList, this);
    else
      task = new Task(taskName, total, session, desktopList, parent);

    updateParents( (QListViewItem *) task, totalDiff, sessionDiff );
    setCurrentItem(task);
    setSelected(task, true);
    // done by updateParents: emit( sessionTimeChanged( sessionDiff, totalDiff ) );
  }
  delete dialog;
}

void TaskView::newSubTask()
{
  QListViewItem *item = currentItem();
  if(!item)
    return;
  newTask(i18n("New Sub Task"), item);
  // newTask will emit( sessionTimeChanged() ).
  item->setOpen(true);
  setRootIsDecorated(true);
}

void TaskView::editTask()
{
  Task *task = (Task *) currentItem();
  if (!task)
  return;

  DesktopListType desktops = task->getDesktops();
  AddTaskDialog *dialog = new AddTaskDialog(i18n("Edit Task"), true, &desktops);
  dialog->setTask(task->name(),
                  task->totalTime(),
                  task->sessionTime());
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
    DesktopListType desktopList;
    dialog->status( &total, &totalDiff, &session, &sessionDiff, &desktopList);

    task->setTotalTime( total);
    task->setSessionTime( session );

    // If all available desktops are checked, disable auto tracking,
    // since it makes no sense to track for every desktop.
    if (desktopList.size() == (unsigned int)desktopCount)
      desktopList.clear();

    task->setDesktopList(desktopList);

    // done by updateParents
    //if( sessionDiff || totalDiff ) {
    //  emit sessionTimeChanged( sessionDiff, totalDiff );
    //}

    // Update the parents for this task.
    updateParents( (QListViewItem *) task, totalDiff, sessionDiff );
    updateTrackers(task, desktopList);

    emit updateButtons();
  }
  delete dialog;
}

void TaskView::updateParents( QListViewItem* task, long totalDiff,
                              long sessionDiff )
{
  QListViewItem *item = task->parent();
  while (item) {
    Task *parentTask = (Task *) item;
    parentTask->setTotalTime(parentTask->totalTime()+totalDiff);
    parentTask->setSessionTime(parentTask->sessionTime()+sessionDiff);
    item = item->parent();
  }
  // only toplevel tasks directly contribute to the statusbar
  // (otherwise subtasks would contribute twice)
  sessionTimeChanged( sessionDiff, totalDiff );
}

void TaskView::deleteTask()
{
  Task *item = ((Task *) currentItem());
  if (item == 0) {
    KMessageBox::information(0,i18n("No task selected"));
    return;
  }

  int response = KMessageBox::Yes;
  if ( _preferences->promptDelete() ) {
      if (item->childCount() == 0) {
          response = KMessageBox::questionYesNo(0,
                  i18n( "Are you sure you want to delete "
                        "the task named\n\"%1\"").arg(item->name()),
                  i18n( "Deleting Task"));
      }
      else {
          response = KMessageBox::questionYesNo(0,
                  i18n( "Are you sure you want to delete the task named"
                        "\n\"%1\"\n" "NOTE: all its subtasks will also "
                        "be deleted!").arg(item->name()),
                  i18n( "Deleting Task"));
      }
  }

  if (response == KMessageBox::Yes) {

    // Remove chilren from the active set of tasks.
    stopChildCounters(item);
    stopTimerFor(item);

    // Stop idle detection if no more counters is running
    if (activeTasks.count() == 0) {
      _idleTimeDetector->stopIdleDetection();
      emit timerInactive();
    }
    emit tasksChanged( activeTasks );

    long sessionTime = item->sessionTime();
    long totalTime   = item->totalTime();
    updateParents( item, -totalTime, -sessionTime );

    DesktopListType desktopList;
    updateTrackers(item, desktopList); // remove from tracker list

    delete item;

    // remove root decoration if there is no more children.
    bool anyChilds = false;
    for(QListViewItem *child=firstChild(); child; child=child->nextSibling()) {
      if (child->childCount() != 0) {
        anyChilds = true;
        break;
      }
    }
    if (!anyChilds) {
      setRootIsDecorated(false);
    }
  }
}

void TaskView::stopChildCounters(Task *item)
{
  for ( QListViewItem *child=item->firstChild();
        child;
        child=child->nextSibling()) {
    stopChildCounters((Task *)child);
  }
  activeTasks.removeRef(item);
}


void TaskView::extractTime(int minutes)
{
  addTimeToActiveTasks(-minutes);
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

#include "taskview.moc"
