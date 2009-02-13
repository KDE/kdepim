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

#include "akonadi-kcal_export.h"

#include <kcal/incidencebase.h>

namespace KCal {

/**
  Helper for type correct assignment of incidence pointers.

  Usage example:
  @code
  KCal::Incidence *currentIncidence; // assume this is set somewhere else
  KCal::Incidence *updatedIncidence; // assume this is set somewhere else

  ::AssignmentVisitor visitor;

  // assign
  if ( !visitor.assign(currentIncidence, updatedIncidence) ) {
    // not of same type
  }
  @endcode

  @author Kevin Krammer \<kevin.krammer@gmx.at\>

  @since 4.3
 */
class AKONADI_KCAL_EXPORT AssignmentVisitor : public IncidenceBase::Visitor
{
  public:
    /**
      Creates a visitor instance.
     */
    AssignmentVisitor();

    /**
      Destroys the instance
     */
    virtual ~AssignmentVisitor();

    /**
      Assigns the @p source to the @p target

      Basically the sub type equivalent of
      @code
      *target = *source
      @endcode

      @param target the instance to assign to
      @param source the instance to assign from

      @return @c false if the two objects are of different type
     */
    bool assign( IncidenceBase *target, const IncidenceBase *source );

    /**
      Tries to assign to the given @p event

      @return @c false if the source passed to assign() is of a different type
     */
    virtual bool visit( Event *event );

    /**
      Tries to assign to the given @p todo

      @return @c false if the source passed to assign() is of a different type
     */
    virtual bool visit( Todo *todo );

    /**
      Tries to assign to the given @p journal

      @return @c false if the source passed to assign() is of a different type
     */
    virtual bool visit( Journal *journal );

    /**
      Tries to assign to the given @p freebusy^

      @return @c false if the source passed to assign() is of a different type
     */
    virtual bool visit( FreeBusy *freebusy );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

    Q_DISABLE_COPY( AssignmentVisitor )
};

}

#endif
// kate: space-indent on; indent-width 2; replace-tabs on;
