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

#ifndef MESSAGECOMPOSER_JOB_P_H
#define MESSAGECOMPOSER_JOB_P_H

#include "job.h"

#include <kmime/kmime_content.h>

namespace MessageComposer {

class Composer;

class JobPrivate
{
  public:
    JobPrivate( Job *qq )
      : composer( 0 )
      , resultContent( 0 )
      , q_ptr( qq )
    {
    }

    virtual ~JobPrivate()
    {
    }

    void init( QObject *parent );
    void doNextSubjob();

    Composer *composer;
    KMime::Content *resultContent;
    KMime::Content::List subjobContents;

    Job *q_ptr;
    Q_DECLARE_PUBLIC( Job )
};

}

#endif
