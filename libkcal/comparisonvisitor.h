/*
  Copyright 2009 Ingo Klöcker <kloecker@kde.org>

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

#ifndef KCAL_COMPARISONVISITOR_H
#define KCAL_COMPARISONVISITOR_H

#include "incidencebase.h"

namespace KCal {

/**
  Helper for type correct comparison of incidences via pointers.

  This class provides a way of correctly comparing one incidence to another,
  given two IncidenceBase derived pointers. It effectively provides a virtual
  comparison method which first type checks the two pointers to ensure they
  reference the same incidence type, before performing the comparison.

  Usage example:
  @code
  KCal::Incidence *incidence;          // assume this is set somewhere else
  KCal::Incidence *referenceIncidence; // assume this is set somewhere else

  KCal::ComparisonVisitor visitor;

  // compare
  if ( visitor.compare( incidence, referenceIncidence ) ) {
    // incidence and referenceIncidence point to identical incidences
  }
  @endcode

  @author Ingo Klöcker <kloecker@kde.org>
 */
class ComparisonVisitor : public IncidenceBase::Visitor
{
  public:
    /**
      Creates a visitor instance.
     */
    ComparisonVisitor();

    /**
      Destroys the instance.
     */
    virtual ~ComparisonVisitor();

    /**
      Compares the incidence referenced by @p incidence to the incidence
      referenced by @p reference. Returns true, if the incidence referenced
      by @p incidence is identical to the incidence referenced by @p reference.
      Also returns true, if @p incidence and @p reference are both @c 0.

      Basically it is a virtual equivalent of
      @code
      *incidence == *reference
      @endcode

      @param incidence pointer to the incidence to compare with the reference incidence
      @param reference pointer to the reference incidence

      @return @c true if the two incidences are identical or both @c 0
     */
    bool compare( IncidenceBase *incidence, const IncidenceBase *reference );

    /**
      Compares the event referenced by @p event to the incidence passed to
      compare().

      @return @c true if the event is identical to the reference incidence
     */
    virtual bool visit( Event *event );

    /**
      Compares the todo referenced by @p todo to the incidence passed to
      compare().

      @return @c true if the todo is identical to the reference incidence
     */
    virtual bool visit( Todo *todo );

    /**
      Compares the journal referenced by @p journal to the incidence passed to
      compare().

      @return @c true if the journal is identical to the reference incidence
     */
    virtual bool visit( Journal *journal );

    /**
      Compares the freebusy object referenced by @p freebusy to the incidence passed to
      compare().

      @return @c true if the freebusy object is identical to the reference incidence
     */
    virtual bool visit( FreeBusy *freebusy );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif // KCAL_COMPARISONVISITOR_H
