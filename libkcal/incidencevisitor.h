#ifndef INCIDENCEVISITOR_H
#define INCIDENCEVISITOR_H
// $Id$
//
// IncidenceVisitor
//

namespace KCal {

class Event;
class Todo;
class Journal;

/**
  This class provides the interface for a visitor of calendar components. It
  serves as base class for concrete visitors, which implement certain actions on
  calendar components. It allows to add functions, which operate on the concrete
  types of calendar components, without changing the calendar component classes.
*/
class IncidenceVisitor
{
  public:
    /** Destruct IncidenceVisitor */
    virtual ~IncidenceVisitor() {}

    /**
      Reimplement this function in your concrete subclass of IncidenceVisitor to perform actions
      on an Event object.
    */
    virtual bool visit(Event *) { return false; }
    /**
      Reimplement this function in your concrete subclass of IncidenceVisitor to perform actions
      on an Todo object.
    */
    virtual bool visit(Todo *) { return false; }
    /**
      Reimplement this function in your concrete subclass of IncidenceVisitor to perform actions
      on an Journal object.
    */
    virtual bool visit(Journal *) { return false; }
    
  protected:
    /** Constructor is protected to prevent direct creation of visitor base class. */
    IncidenceVisitor() {}
};

};

#endif
