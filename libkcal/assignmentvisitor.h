/*
    Copyright (c) 2009 Kevin Krammer <kevin.krammer@gmx.at>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KCAL_ASSIGNMENTVISITOR_H
#define KCAL_ASSIGNMENTVISITOR_H

#include "incidencebase.h"

namespace KCal {

/**
  Helper for type correct assignment of incidences via pointers.

  This class provides a way of correctly assigning one incidence to another,
  given two IncidenceBase derived pointers. It effectively provides a virtual
  assignment method which first type checks the two pointers to ensure they
  reference the same incidence type, before performing the assignment.

  Usage example:
  @code
  KCal::Incidence *currentIncidence; // assume this is set somewhere else
  KCal::Incidence *updatedIncidence; // assume this is set somewhere else

  KCal::AssignmentVisitor visitor;

  // assign
  if ( !visitor.assign(currentIncidence, updatedIncidence) ) {
    // not of same type
  }
  @endcode

  @author Kevin Krammer \<kevin.krammer@gmx.at\>
 */
class AssignmentVisitor : public IncidenceBase::Visitor
{
  public:
    /**
      Creates a visitor instance.
     */
    AssignmentVisitor();

    /**
      Destroys the instance.
     */
    virtual ~AssignmentVisitor();

    /**
      Assigns the incidence referenced by @p source to the incidence referenced
      by @p target, first ensuring that the @p source incidence can be cast to
      the same class as the @p target incidence.

      Basically it is a virtual equivalent of
      @code
      *target = *source
      @endcode

      @param target pointer to the instance to assign to
      @param source pointer to the instance to assign from

      @return @c false if the two objects are of different type
     */
    bool assign( IncidenceBase *target, const IncidenceBase *source );

    /**
      Tries to assign to the given @p event, using the source passed to
      assign().

      @return @c false if the source passed to assign() is not an Event
     */
    virtual bool visit( Event *event );

    /**
      Tries to assign to the given @p todo, using the source passed to
      assign().

      @return @c false if the source passed to assign() is not a Todo
     */
    virtual bool visit( Todo *todo );

    /**
      Tries to assign to the given @p journal, using the source passed to
      assign().

      @return @c false if the source passed to assign() is not a Journal
     */
    virtual bool visit( Journal *journal );

    /**
      Tries to assign to the given @p freebusy, using the source passed to
      assign().

      @return @c false if the source passed to assign() is not a FreeBusy
     */
    virtual bool visit( FreeBusy *freebusy );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
// kate: space-indent on; indent-width 2; replace-tabs on;
