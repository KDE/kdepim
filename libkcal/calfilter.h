// $Id$
//
// CalFilter - filter for calendar items
//

#ifndef _CALFILTER_H
#define _CALFILTER_H

#include <qstring.h>
#include <qlist.h>

#include "event.h"
#include "todo.h"

namespace KCal {

/**
  Filter for calendar objects.
*/
class CalFilter {
  public:
    /** Construct filter. */
    CalFilter();
    /** Construct filter with name */
    CalFilter(const QString &name);
    /** Destruct filter. */
    ~CalFilter();
    
    /**
      Set name of filter.
    */
    void setName(const QString &name) { mName = name; }
    /**
      Return name of filter.
    */
    QString name() const { return mName; }
    
    /**
      Apply filter to eventlist, all events not matching filter criterias are
      removed from the list.
    */
    void apply(QList<Event> *eventlist);
    
    /**
      Apply filter to todolist, all todos not matching filter criterias are
      removed from the list.
    */
    void apply(QList<Todo> *todolist);
    
    /**
      Apply filter criteria on the specified event. Return true, if event passes
      criteria, otherwise return false.
    */
    bool filterEvent(Event *);
    
    /**
      Apply filter criteria on the specified todo. Return true, if event passes
      criteria, otherwise return false.
    */
    bool filterTodo(Todo *);
    
    /**
      Apply filter criteria on the specified incidence. Return true, if event passes
      criteria, otherwise return false.
    */
    bool filterIncidence(Incidence *);
    
    /**
      Enable or disable filter.
    */
    void setEnabled(bool);
    /**
      Return wheter the filter is enabled or not.
    */
    bool isEnabled();


    /**
      Set list of categories, which is used for showing/hiding categories of
      events.
      See related functions.
    */
    void setCategoryList(const QStringList &);
    /**
      Return category list, used for showing/hiding categories of events.
      See related functions.
    */
    QStringList categoryList();
    
    enum { HideRecurring = 1, HideCompleted = 2, ShowCategories = 4 };
    
    /**
      Set criteria, which have to be fulfilled by events passing the filter.
    */
    void setCriteria(int);
    /**
      Get inclusive filter criteria.
    */
    int criteria();
    
  private:
    QString mName;

    int mCriteria;
    
    bool mEnabled;
    
    QStringList mCategoryList;
};

}

#endif /*  _CALFILTER_H  */
