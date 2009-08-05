/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef MESSAGECOMPOSER_JOBBASE_H
#define MESSAGECOMPOSER_JOBBASE_H

#include "messagecomposer_export.h"

#include <QtCore/QList>

#include <KDE/KCompositeJob>

namespace MessageComposer {

class GlobalPart;
class JobBasePrivate;

/**
  A dummy abstract class defining some errors pertaining to the Composer.
  It is meant to be subclassed.
*/
class MESSAGECOMPOSER_EXPORT JobBase : public KCompositeJob
{
  Q_OBJECT

  public:
    typedef QList<JobBase*> List;

    enum Error
    {
      BugError = UserDefinedError + 1,
      IncompleteError,
      UserCancelledError,
      UserError = UserDefinedError + 42
    };

    explicit JobBase( QObject *parent = 0 );
    virtual ~JobBase();

    // asserts if no Composer parent
    GlobalPart *globalPart();

  protected:
    JobBasePrivate *const d_ptr;
    JobBase( JobBasePrivate &dd, QObject *parent );

  private:
    Q_DECLARE_PRIVATE( JobBase )
};

} // namespace MessageComposer

#endif
