#include <algorithm>            // std::find

#include "desktoptracker.h"

DesktopTracker::DesktopTracker ()
{
  // Setup desktop change handling
  connect( &kWinModule, SIGNAL( currentDesktopChanged(int) ),
           this, SLOT( handleDesktopChange(int) ));

  _desktopCount = kWinModule.numberOfDesktops();
  _previousDesktop = kWinModule.currentDesktop()-1;
  // TODO: removed? fixed by Lubos?
  // currentDesktop will return 0 if no window manager is started
  if( _previousDesktop < 0 ) _previousDesktop = 0;
}

void DesktopTracker::handleDesktopChange(int desktop)
{
  desktop--; // desktopTracker starts with 0 for desktop 1
  // notify start all tasks setup for running on desktop
  TaskVector::iterator it;

  // stop trackers for _previousDesktop
  TaskVector tv = desktopTracker[_previousDesktop];
  for (it = tv.begin(); it != tv.end(); it++) {
    emit leftActiveDesktop(*it);
  }

  // start trackers for desktop
  tv = desktopTracker[desktop];
  for (it = tv.begin(); it != tv.end(); it++) {
    emit reachedtActiveDesktop(*it);
  }
  _previousDesktop = desktop;

  // emit updateButtons();
}

void DesktopTracker::startTracking()
{
  int currentDesktop = kWinModule.currentDesktop() -1;
  // TODO: removed? fixed by Lubos?
  // currentDesktop will return 0 if no window manager is started
  if ( currentDesktop < 0 ) currentDesktop = 0;

  TaskVector &tv = desktopTracker[ currentDesktop ];
  TaskVector::iterator tit = tv.begin();
  while(tit!=tv.end()) {
    emit reachedtActiveDesktop(*tit);
    tit++;
  }
}

void DesktopTracker::registerForDesktops( Task* task, DesktopList desktopList)
{
  // if no desktop is marked, disable auto tracking for this task
  if (desktopList.size()==0) {
    for (int i=0; i<16; i++) {
      TaskVector *v = &(desktopTracker[i]);
      TaskVector::iterator tit = std::find(v->begin(), v->end(), task);
      if (tit != v->end())
        desktopTracker[i].erase(tit);
        // if the task was priviously tracking this desktop then
        // emit a signal that is not tracking it any more
        if( i == kWinModule.currentDesktop() -1)
          emit leftActiveDesktop(task);
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
        if (tit != v.end()) // not in start vector any more
        {
          v.erase(tit); // so we delete it from desktopTracker
          // if the task was priviously tracking this desktop then
          // emit a signal that is not tracking it any more
          if( i == kWinModule.currentDesktop() -1)
            emit leftActiveDesktop(task);
        }
      }
    }
    startTracking();
  }
}

void DesktopTracker::printTrackers() {
  TaskVector::iterator it;
  for (int i=0; i<16; i++) {
    TaskVector& start = desktopTracker[i];
    it = start.begin();
    while (it != start.end()) {
      it++;
    }
  }
}
#include "desktoptracker.moc"
