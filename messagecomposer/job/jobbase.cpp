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

#include "jobbase.h"

#include "composer/composer.h"
#include "jobbase_p.h"

#include <KDebug>

using namespace MessageComposer;

JobBase::JobBase( QObject *parent )
  : KCompositeJob( parent )
  , d_ptr( new JobBasePrivate( this ) )
{
}

JobBase::JobBase( JobBasePrivate &dd, QObject *parent )
  : KCompositeJob( parent )
  , d_ptr( &dd )
{
}

JobBase::~JobBase()
{
  delete d_ptr;
}

GlobalPart *JobBase::globalPart()
{
  for( QObject *obj = this; obj != 0; obj = obj->parent() ) {
    Composer *composer = qobject_cast<Composer*>( obj );
    if( composer ) {
      return composer->globalPart();
    }
  }

  kFatal() << "Job is not part of a Composer.";
  return 0;
}


#include "moc_jobbase.cpp"
