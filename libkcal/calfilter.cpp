// $Id$
//
// CalFilter implementation
//

#include "kdebug.h"

#include "calfilter.h"

using namespace KCal;

CalFilter::CalFilter()
{
  mEnabled = true;
  mCriteria = 0;
}

CalFilter::CalFilter(const QString &name)
{
  mName = name;
}

CalFilter::~CalFilter()
{
}

void CalFilter::apply(QList<Event> *eventlist)
{
  if (!mEnabled) return;

//  kdDebug() << "CalFilter::apply()" << endl;

  Event *event = eventlist->first();
  while(event) {
    if (!filterEvent(event)) {
      eventlist->remove();
      event = eventlist->current();
    } else {
      event = eventlist->next();
    }
  }

//  kdDebug() << "CalFilter::apply() done" << endl;
}

// TODO: avoid duplicating apply() code
void CalFilter::apply(QList<Todo> *eventlist)
{
  if (!mEnabled) return;

//  kdDebug() << "CalFilter::apply()" << endl;

  Todo *event = eventlist->first();
  while(event) {
    if (!filterTodo(event)) {
      eventlist->remove();
      event = eventlist->current();
    } else {
      event = eventlist->next();
    }
  }

//  kdDebug() << "CalFilter::apply() done" << endl;
}

bool CalFilter::filterEvent(Event *event)
{
//  kdDebug() << "CalFilter::filterEvent(): " << event->getSummary() << endl;

  if (mCriteria & HideRecurring) {
    if (event->recurrence()->doesRecur()) return false;
  }

  return filterIncidence(event);
}

bool CalFilter::filterTodo(Todo *todo)
{
//  kdDebug() << "CalFilter::filterEvent(): " << event->getSummary() << endl;

  if (mCriteria & HideCompleted) {
    if (todo->isCompleted()) return false;
  }

  return filterIncidence(todo);
}

bool CalFilter::filterIncidence(Incidence *incidence)
{
//  kdDebug() << "CalFilter::filterEvent(): " << event->getSummary() << endl;

  if (mCriteria & ShowCategories) {
    for (QStringList::Iterator it = mCategoryList.begin();
         it != mCategoryList.end(); ++it ) {
      QStringList incidenceCategories = incidence->categories();
      for (QStringList::Iterator it2 = incidenceCategories.begin();
           it2 != incidenceCategories.end(); ++it2 ) {
        if ((*it) == (*it2)) {
          return true;
        }
      }
    }
    return false;
  } else {
    for (QStringList::Iterator it = mCategoryList.begin();
         it != mCategoryList.end(); ++it ) {
      QStringList incidenceCategories = incidence->categories();
      for (QStringList::Iterator it2 = incidenceCategories.begin();
           it2 != incidenceCategories.end(); ++it2 ) {
        if ((*it) == (*it2)) {
          return false;
        }
      }
    }
    return true;
  }
    
//  kdDebug() << "CalFilter::filterEvent(): passed" << endl;
  
  return true;
}

void CalFilter::setEnabled(bool enabled)
{
  mEnabled = enabled;
}

bool CalFilter::isEnabled()
{
  return mEnabled;
}

void CalFilter::setCriteria(int criteria)
{
  mCriteria = criteria;
}

int CalFilter::criteria()
{
  return mCriteria;
}

void CalFilter::setCategoryList(const QStringList &categoryList)
{
  mCategoryList = categoryList;
}

QStringList CalFilter::categoryList()
{
  return mCategoryList;
}
